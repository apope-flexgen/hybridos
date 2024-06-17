// service_test.go
package fimsapi

import (
	"testing"

	fims "github.com/flexgen-power/hybridos/fims/go_fims"
	"github.com/stretchr/testify/assert"
)

func TestNewService(t *testing.T) {
	service := NewService()
	assert.NotNil(t, service)
	assert.NotNil(t, service.conn)
	assert.NotNil(t, service.Router)
}

func TestService_Close(t *testing.T) {
	service := NewService()
	service.conn = &fims.Fims{}
	service.Configure("testService", "uri1", "uri2")

	service.Close()
	assert.False(t, service.conn.Connected())
}

func TestService_GetConnection(t *testing.T) {
	service := NewService()
	conn := service.GetConnection()
	assert.Equal(t, service.conn, conn)
}
