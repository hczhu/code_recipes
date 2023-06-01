package etcd_leader_election

import (
	"context"
	"errors"
	"log"
	"sync/atomic"
	"time"

	clientv3 "go.etcd.io/etcd/client/v3"
	"go.etcd.io/etcd/client/v3/concurrency"
)

type closable interface {
	Close() error
}

type Config struct {
	EtcdSessionTTL time.Duration
	ElectionPrefix	 string
	EtcdEndpoints []string
	// This is used as the value of the ETCD leader election key for the caller and also logging.
	InstanceId string
}

type LeaderElection struct {
	etcdClient *clientv3.Client
	etcdSession *concurrency.Session
	etcdElection *concurrency.Election
	cancelCampaign context.CancelFunc
	instanceId string
	isClosed *atomic.Bool
	cancelCh chan struct{}
	isLeader *atomic.Bool

	// Once a caller is elected as a leader, this channel will be closed.
	// The leader won't involuntarily lose the leadership as long as its etcd session is valid.
	// A session expires when the etcd servers don't receive a heartbeat from the client within the session TTL.
	// The leader doesn't need to proactively send heartbeats to the etcd servers. The etcd client library will do it automatically.
	BecomeLeaderCh chan struct{}
	// An error can happen before or after the caller becomes a leader.
	// If an error happens before the caller becomes a leader, the caller will never become a leader.
	// If an error happens after the caller becomes a leader, the caller is not the leader anymore.
	// When an error happens, the LeaderElection object becomes invalid and should be closed.
	// NB: LeaderElection.Close() also causes an error.
	// NB: only one error will be sent to this channel.
	ErrorCh chan error
}

// Return true if the caller becomes the leader.
// Return false if any error happens.
// After an error, the LeaderElection object becomes invalid and should be closed.
// This method can be called concurrently.
func (l *LeaderElection) BlockingWaitForLeadership() bool {
	if l.isClosed.Load() {
		return false
	}
	select {
	case<-l.BecomeLeaderCh:
		return true
	case<-l.cancelCh:
		return false
	}
}


// Will resign the leadership (if the caller is elected) and close the etcd session.
// The LeaderLelection object will be invalid after this call.
// Close() must be called if the caller doesn't want to be the leader anymore.
func (l *LeaderElection) Close(logger *log.Logger) {
	if l.isClosed.Swap(true) {
		logger.Println(l.instanceId, ": Already closed.")
		return 
	}
	logger.Println(l.instanceId, ": Canceling the campaign...")
	// This will resign the leadership if caller is already the leader but
	// its campaign go routine is still in Campaign().
	l.cancelCampaign()
	if l.isLeader.Load() {
		logger.Println(l.instanceId, ": Resigning the leadership...")
		ctx, cancel := context.WithTimeout(context.Background(), 3 * time.Second)
		defer cancel()
		l.etcdElection.Resign(ctx)
	}
	logger.Println(l.instanceId, ": Closing the etcd session...")
	l.etcdSession.Close()
	logger.Println(l.instanceId, ": Closing the etcd client...")
	l.etcdClient.Close()
}

func createEtcdClient(etcdEndpoints []string) (*clientv3.Client, error) {
    dialTimeout := time.Duration(10 * time.Second)
	ctx, cancel := context.WithTimeout(context.Background(), dialTimeout)
	defer cancel()
	return clientv3.New(clientv3.Config{
    		Endpoints:   etcdEndpoints,
    		DialTimeout: dialTimeout,
    		Context:	 ctx,
	})	
}
func createEtcdSession(client *clientv3.Client, config *Config) (*concurrency.Session, error) {
	ctx, cancel := context.WithCancel(context.Background())
	go func () {
    	sessionCreationTimeout := time.Duration(8 * time.Second)
		select {
		case <-ctx.Done():
			cancel()
			break
		case <-time.After(sessionCreationTimeout):
			cancel()
		}
	}()
	// NB: This call will be blocked indefinitely if the etcd servers are not reachable.
    return concurrency.NewSession(
		client,
		concurrency.WithTTL(int(config.EtcdSessionTTL.Seconds())),
		concurrency.WithContext(ctx),
	)
}

// Call LeaderElection.Close() to cancel the election participation or yield the leadership.
func StartLeaderElectionAsync(config Config, logger *log.Logger) (LeaderElection, error){
	toClose := make([]closable, 0)
	defer func() {
		for i := int(len(toClose)) - 1; i >= 0; i-- {
			toClose[i].Close()
		}	
	}()

	logger.Printf("Establishing connection to etcd endpoint: %+v\n", config.EtcdEndpoints)
	client, err := createEtcdClient(config.EtcdEndpoints)

	if err != nil {
		logger.Printf("Failed to created an ETCD client with error: %v\n", err)
		return LeaderElection{}, err
	}
	toClose = append(toClose, client)

	logger.Println("Creating an Etcd session...")
    session, err := createEtcdSession(client, &config)
	if err != nil {
		logger.Printf("Failed to created an ETCD session with error: %v\n", err)
		return LeaderElection{}, err
	}
	toClose = append(toClose, session)
	logger.Println("Etcd session is created successfully.")

	election := concurrency.NewElection(session, config.ElectionPrefix)
	campaignCtx, cancelCampaign := context.WithCancel(context.Background())
	becomeLeaderCh := make(chan struct{})
	errorCh := make(chan error, 1)
	cancelCh := make(chan struct{})
	isLeader := atomic.Bool{}
	go func(campaignErrorCh chan error, becomeLeaderCh chan struct{}) {
		logger.Printf("%s: Obtaining leadership with etcd prefix: %s\n", config.InstanceId, config.ElectionPrefix)
		// This will block until the caller becomes the leader, an error occurs, or the context is cancelled.
		err := election.Campaign(campaignCtx, config.InstanceId)
		if err == nil {
			logger.Printf("%s: I am the leader for election prefix: %s\n", config.InstanceId, config.ElectionPrefix)
			isLeader.Store(true)
			// The leader will hold the leadership until it resigns or the session expires. The session will keep alive by the underlying etcd client
			// automatically sending heartbeats to the etcd server. The session will expire if the etcd server does not receive heartbeats from the client within the session TTL.
			close(becomeLeaderCh)
			logger.Printf("%s: Waiting for session done.\n", config.InstanceId)
			<-session.Done()
			logger.Printf("%s: The session is done. I am not the leader anymore for election prefix: %s\n", config.InstanceId, config.ElectionPrefix)
			errorCh <- errors.New("the session is done. I am not the leader anymore")
			logger.Printf("%s: Closing the cancel chan.\n", config.InstanceId)
			close(cancelCh)
		} else {
			logger.Printf("%s: Campaign() returned an error: %+v.\n", config.InstanceId, err)
			errorCh <- err
			logger.Printf("%s: Closing the cancel chan.\n", config.InstanceId)
			close(cancelCh)
		}
		logger.Printf("%s: Campaign go routine exit.\n", config.InstanceId)
	}(errorCh, becomeLeaderCh)

	toClose = toClose[:0]
	return LeaderElection{
		etcdClient: client,
		etcdSession: session,
		etcdElection: election,
		cancelCampaign: cancelCampaign,
		instanceId: config.InstanceId,
		isClosed: &atomic.Bool{},
		isLeader: &isLeader,
		cancelCh: cancelCh,

		BecomeLeaderCh: becomeLeaderCh,
		ErrorCh: errorCh,
	}, nil
}
