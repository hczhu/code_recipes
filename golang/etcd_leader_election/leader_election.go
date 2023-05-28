package etcd_leader_election

import (
	"context"
	"log"
	"time"

	_ "go.etcd.io/etcd/client/v3"
	"go.etcd.io/etcd/client/v3/concurrency"
	_ "go.etcd.io/etcd/client/v3/concurrency"
	_ "go.etcd.io/etcd/tests/v3/framework/integration"
	// clientv3 "go.etcd.io/etcd/client/v3"
)

type closable interface {
	Close() error
}

func startLeaderElection(rmConfig *ruleManagerConfig, logger log.Logger) (rmLeaderElection, error){
	toClose := make([]closable, 0)
	defer func() {
		for _, c := range toClose {
			c.Close()
		}	
	}()

	level.Info(logger).Log("msg", "Establishing connection to etcd...")
	client, err := func () (*clientv3.Client, error) {
		timeout := time.Duration(5 * time.Second)
		ctx, cancel := context.WithTimeout(context.Background(), timeout)
		defer cancel()
		return clientv3.New(clientv3.Config{
    		Endpoints:   []string{rmConfig.etcdEndpoint},
    		DialTimeout: timeout,
    		Context:	 ctx,
    	})
	}()
	if err != nil {
		level.Error(logger).Log(
			"msg", "Failed to created an ETCD client.",
			"error", err,
		)
		return rmLeaderElection{}, err
	}
	level.Info(logger).Log("msg", "Etcd connection successful")
	// If this pod dies unexpectedly and ungracefully, the session will expire after the TTL
	// and another pod will be elected as the new leader.
    session, err := concurrency.NewSession(client, concurrency.WithTTL(
		int(time.Duration(rmConfig.etcdSessionTTL).Seconds())),
	)
	if err != nil {
		level.Error(logger).Log(
			"msg", "Failed to created an ETCD session with error.",
			"error", err,
		)
		client.Close()
		return rmLeaderElection{}, err
	}
	level.Info(logger).Log("msg", "Etcd session created successfully")

	etcdPrefix := "/m3/" + rmConfig.etcdLeaderElectionKey
	if rmConfig.dryRun || rmConfig.dryRunWithWriteBack {
		etcdPrefix += "-dryrun"
	}
	election := concurrency.NewElection(session, etcdPrefix)
	level.Info(logger).Log("msg", "Obtaining leadership with etcd prefix: "+etcdPrefix)
	// This will block until the caller becomes the leader or an error occurs.
	err = election.Campaign(context.Background(), rmConfig.podId)
	if err != nil {
		level.Error(logger).Log(
			"msg", "Failed to call Campain for leadership.",
			"error", err,
		)
		session.Close()
		client.Close()
		return rmLeaderElection{}, err
	}

	level.Info(logger).Log("msg", "I am the leader - "+podId)
	rmConfig.election = election
	defer client.Close()
}
 