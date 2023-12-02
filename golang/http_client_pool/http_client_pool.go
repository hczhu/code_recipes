package http_client_pool

import (
	"io"
	"log"
	"net/http"
	"sync"
	"time"
)

type Options struct {
	HttpRequestTimeout time.Duration
	NumClients         int
	NumGoRoutines      int
	ItemQueueSize	   int
}

func NewOptions() Options {
	return Options{
		HttpRequestTimeout: 5 * time.Second,
		NumClients: 	   2,
		NumGoRoutines:     5,
		ItemQueueSize:    10,
	}
}

func getHttpClient(timeout time.Duration) *http.Client {
	t := http.DefaultTransport.(*http.Transport).Clone()
	t.MaxIdleConns = 10
	t.MaxConnsPerHost = 10
	t.MaxIdleConnsPerHost = 10
	return &http.Client{
		Timeout:   timeout,
		Transport: t,
	}
}

type HttpClientPoolItem func (*http.Client)

type HttpClientPool struct {
	opts    Options
	clients []*http.Client
	stopCh  chan struct{}
	itemch  chan HttpClientPoolItem
	// wg is used to wait for all goroutines to finish
	wg 	    sync.WaitGroup
	// pendingItemsWg is used to wait for all pending items to be processed
	pendingItemsWg  sync.WaitGroup
}

func NewHttpClientPool(options Options) *HttpClientPool {
	clients := make([]*http.Client, options.NumClients)
	for i := 0; i < options.NumClients; i++ {
		clients[i] = getHttpClient(options.HttpRequestTimeout)
	}
	return &HttpClientPool{
		opts:    options,
		clients: clients,
		stopCh:  make(chan struct{}),
		itemch:  make(chan HttpClientPoolItem, options.ItemQueueSize),
	}
}

func (p *HttpClientPool) Start() {
	for i := 0; i < p.opts.NumGoRoutines; i++ {
		p.wg.Add(1)
		go func(client *http.Client) {
			defer p.wg.Done()
			for {
				select {
				case <-p.stopCh:
					return
				case item := <-p.itemch:
					item(client)
					p.pendingItemsWg.Done()
				}
			}
		}(p.clients[i % p.opts.NumClients])
	}
}

func (p *HttpClientPool) Stop() {
	close(p.stopCh)
	p.wg.Wait()
}

func (p *HttpClientPool) Submit(item HttpClientPoolItem) {
	p.pendingItemsWg.Add(1)
	p.itemch <- item
}

func (p *HttpClientPool) WaitForPendingItems() {
	p.pendingItemsWg.Wait()
}

func CrawlUrl(client *http.Client, url string) (string, error) {
	resp, err := client.Get(url)
	if err != nil {
		log.Default().Println("Get URL: ", url, " Error: ", err)
		return "", err
	}
	body, err := io.ReadAll(resp.Body)
	defer resp.Body.Close()
	if err != nil {
		log.Default().Println("Read body error: ", err ," from URL: ", url)
		return "", err
	}
	return string(body), nil
}