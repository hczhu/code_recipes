package etcd_leader_election

import (
	"context"
	"log"
	"time"

	clientv3 "go.etcd.io/etcd/client/v3"
	"go.etcd.io/etcd/client/v3/concurrency"
	_ "go.etcd.io/etcd/tests/v3/framework/integration"
)

type closable interface {
	Close() error
}

type Config struct {
	EtcdSessionTTL time.Duration
	ElectionPrefix	 string
	EtcdEndpoint string
	InstanceId string

	EtcdClient *clientv3.Client
}

type LeaderElection struct {
	etcdClient *clientv3.Client
	etcdSession *concurrency.Session
	etcdElection *concurrency.Election
	cancelCampaign context.CancelFunc
	instanceId string

	CampaignErrorCh <-chan error
	BecomeLeaderCh <-chan struct{}
	LoseLeadershipCh <-chan struct{}
}

// Will resign the leadership (if the caller is elected) and close the etcd session.
// "l" will be invalid after this call.
func (l *LeaderElection) Close(logger *log.Logger) {
	logger.Println(l.instanceId, ": Canceling the campaign...")
	l.cancelCampaign()
	logger.Println(l.instanceId, ": Resigning the election...")
	l.etcdElection.Resign(context.Background())
	logger.Println(l.instanceId, ": Closing the etcd session...")
	l.etcdSession.Close()
	logger.Println(l.instanceId, ": Closing the etcd client...")
	l.etcdClient.Close()
}

func createEtcdClient(etcdEndpoint string) (*clientv3.Client, error) {
	timeout := time.Duration(5 * time.Second)
	ctx, cancel := context.WithTimeout(context.Background(), timeout)
	defer cancel()
	client, err := clientv3.New(clientv3.Config{
    		Endpoints:   []string{etcdEndpoint},
    		DialTimeout: timeout,
    		Context:	 ctx,
	})	
	return client, err
}

func StartLeaderElectionAsync(config Config, logger *log.Logger) (LeaderElection, error){
	toClose := make([]closable, 0)
	defer func() {
		for i := int(len(toClose)) - 1; i >= 0; i-- {
			toClose[i].Close()
		}	
	}()

	client := config.EtcdClient
	var err error = nil
	if client == nil {
		logger.Printf("Establishing connection to etcd endpoint: %s\n", config.EtcdEndpoint)
		client, err = createEtcdClient(config.EtcdEndpoint)
	}
	if err != nil {
		logger.Printf("Failed to created an ETCD client with error: %v\n", err)
		return LeaderElection{}, err
	}
	toClose = append(toClose, client)
	logger.Println("Etcd connection is established successfully.")

	// If this caller exits without calling Resign() or Close(), the session will expire after the TTL
	// and the leadership will be lost, if this caller was the leader.
    session, err := concurrency.NewSession(client, concurrency.WithTTL(
		int(config.EtcdSessionTTL.Seconds()),
	))
	if err != nil {
		logger.Printf("Failed to created an ETCD session with error: %v\n", err)
		return LeaderElection{}, err
	}
	toClose = append(toClose, session)
	logger.Println("Etcd session is created successfully.")

	election := concurrency.NewElection(session, config.ElectionPrefix)
	campaignCtx, cancelCampaign := context.WithCancel(context.Background())
	campaignErrorCh := make(chan error)
	becomeLeaderCh := make(chan struct{})
	go func(campaignErrorCh chan error, becomeLeaderCh chan struct{}) {
		for campaignCtx.Err() == nil {
			logger.Printf("%s: Obtaining leadership with etcd prefix: %s\n", config.InstanceId, config.ElectionPrefix)
			// This will block until the caller becomes the leader, an error occurs, the context is cancelled.
			err := election.Campaign(campaignCtx, config.ElectionPrefix)
			if err == nil {
				logger.Printf("%s: I am the leader for election prefix: %s\n", config.InstanceId, config.ElectionPrefix)
				becomeLeaderCh <- struct{}{}
				// The leader will hold the leadership until it resigns or the session expires. The session will keep alive by the underlying etcd client
				// automatically send heartbeats to the etcd server. The session will expire if the etcd server does not receive heartbeats from the client within the session TTL.
				break
			} else if campaignCtx.Err() == nil {
				// When the leader changes, the blocking Campaign() call will return with an error, "lost watcher waiting for delete".
				// The reason should be the watched key has changed. This is the most common error here.
				logger.Printf("%s: Campaign() returned an error: %+v. Will retry.\n", config.InstanceId, err)
				campaignErrorCh <- err
			}
		}
	}(campaignErrorCh, becomeLeaderCh)

	toClose = toClose[:0]
	return LeaderElection{
		etcdClient: client,
		etcdSession: session,
		etcdElection: election,
		cancelCampaign: cancelCampaign,
		instanceId: config.InstanceId,

		CampaignErrorCh: campaignErrorCh,
		BecomeLeaderCh: becomeLeaderCh,
		LoseLeadershipCh: session.Done(),
	}, nil
}
 
