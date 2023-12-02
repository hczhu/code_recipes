package http_client_pool

import (
	"net/http"
	"sync/atomic"
	"testing"

	"github.com/stretchr/testify/require"
)

func TestEmptyWork(t *testing.T) {
	p := NewHttpClientPool(NewOptions())
	p.Start()
	defer p.Stop()
}

func TestSimple(t *testing.T) {
	p := NewHttpClientPool(NewOptions())
	p.Start()
	var called atomic.Int32
	for i := 0; i < 20; i++ {
		ii := i
		p.Submit(func (client *http.Client) {
			t.Log("Work item from call #", ii)
			called.Add(1)
		})
	}
	p.WaitForPendingItems()
	defer p.Stop()
	require.Equal(t, int32(20), called.Load())
}

func TestCrawlTickerTick(t *testing.T) {
	p := NewHttpClientPool(NewOptions())
	p.Start()
	tickers := []string{
		"meta", "goog", "amzn", "fb", "aapl", "msft", "nflx", "tsla", "twtr", "snap", "uber",
	}
	tickerSet := make(map[string]struct{})
	for _, k := range tickers {
		tk := k
		p.Submit(func (client *http.Client) {
			t.Log("Crwaling ticker", tk)
			require.NotContains(t, tickerSet, tk)
			tickerSet[tk] = struct{}{}
			content, err := CrawlUrl(client, "https://api.tickertick.com/feed?query=z:" + tk)
			require.NoError(t, err)
			prefix := content
			if len(prefix) > 50 {
				prefix = prefix[:50]
			}
			t.Log("Got content of ", len(content), "bytes: ", prefix)
		})
	}
	p.WaitForPendingItems()
	defer p.Stop()
	require.Equal(t, len(tickers), len(tickerSet))
}

