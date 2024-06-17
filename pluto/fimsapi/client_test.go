// client_test.go
package fimsapi

import (
	"testing"

	fims "github.com/flexgen-power/hybridos/fims/go_fims"
	"github.com/stretchr/testify/assert"
)

type FakeService struct {
	conn *fims.Fims
}

func (fs *FakeService) Close() {
	// Simulate closing the connection
	fs.conn = nil
}

func (fs *FakeService) SendSet(replyTo string, body interface{}) error {
	// Simulate sending a message
	return nil
}

func NewFakeService() *FakeService {
	return &FakeService{
		conn: &fims.Fims{},
	}
}

func TestNewClient(t *testing.T) {
	client := NewClient()
	assert.NotNil(t, client)
	assert.NotNil(t, client.service)
}
