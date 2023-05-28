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
}

type LeaderElection struct {
	etcdClient *clientv3.Client
	etcdSession *concurrency.Session
	etcdElection *concurrency.Election
	cancelCampaign context.CancelFunc

	CampaignErrorCh <-chan error
	BecomeLeaderCh <-chan struct{}
	LoseLeadershipCh <-chan struct{}
}

// Will resign the leadership (if the caller is elected) and close the etcd session.
// "l" will be invalid after this call.
func (l *LeaderElection) Close(logger log.Logger) {
	logger.Println("Canceling the campaign...")
	l.cancelCampaign()
	logger.Println("Resigning the election...")
	l.etcdElection.Resign(context.Background())
	logger.Println("Closing the etcd session...")
	l.etcdSession.Close()
	logger.Println("Closing the etcd client...")
	l.etcdClient.Close()
}

func StartLeaderElectionAsync(config Config, logger log.Logger) (LeaderElection, error){
	toClose := make([]closable, 0)
	defer func() {
		for i := int(len(toClose)) - 1; i >= 0; i-- {
			toClose[i].Close()
		}	
	}()

	logger.Println("Establishing connection to etcd...")
	client, err := func () (*clientv3.Client, error) {
		timeout := time.Duration(5 * time.Second)
		ctx, cancel := context.WithTimeout(context.Background(), timeout)
		defer cancel()
		return clientv3.New(clientv3.Config{
    		Endpoints:   []string{config.EtcdEndpoint},
    		DialTimeout: timeout,
    		Context:	 ctx,
    	})
	}()
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
		logger.Print("Failed to created an ETCD session with error: %v\n", err)
		return LeaderElection{}, err
	}
	toClose = append(toClose, session)
	logger.Println("Etcd session is created successfully.")

	election := concurrency.NewElection(session, config.ElectionPrefix)
	campaignCtx, cancelCampaign := context.WithCancel(context.Background())
	campaignErrorCh := make(chan error)
	becomeLeaderCh := make(chan struct{})
	go func(campaignErrorCh chan error, becomeLeaderCh chan struct{}) {
		logger.Printf("Obtaining leadership with etcd prefix: %s\n", config.ElectionPrefix)
	    // This will block until the caller becomes the leader, an error occurs, the context is cancelled.
		err := election.Campaign(campaignCtx, config.ElectionPrefix)
		if err == nil {
	 		logger.Printf("I am the leader with prefix: %s\n", config.ElectionPrefix)
			becomeLeaderCh <- struct{}{}
		} else {
			logger.Printf("Campaign() returned an error: %+v\n", err)
			campaignErrorCh <- err
		}
	}(campaignErrorCh, becomeLeaderCh)

	toClose = toClose[:0]
	return LeaderElection{
		etcdClient: client,
		etcdSession: session,
		etcdElection: election,
		cancelCampaign: cancelCampaign,

		CampaignErrorCh: campaignErrorCh,
		BecomeLeaderCh: becomeLeaderCh,
		LoseLeadershipCh: session.Done(),
	}, nil
}
 
