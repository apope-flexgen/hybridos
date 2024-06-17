// service.go sets up handlers and routes for fims messages.
package fimsapi

import (
	"fmt"

	log "github.com/flexgen-power/go_flexgen/logger"
	fims "github.com/flexgen-power/hybridos/fims/go_fims"
	mux "github.com/flexgen-power/hybridos/go_flexgen/mux"
)

// Create an object to hold our fims connection and our fims mux router.
type Service struct {
	conn   *fims.Fims
	Router *mux.Router
}

// Return a new empty service object.
func NewService() *Service {
	return &Service{
		conn:   &fims.Fims{},
		Router: mux.NewRouter(),
	}
}

// Configure our fims service connection.
func (s *Service) Configure(service string, uris ...string) error {
	var conn fims.Fims

	// Configure a new FIMS connection.
	conn, err := fims.Configure(service, uris...)
	if err != nil {
		return fmt.Errorf("configuring FIMS connection: %v", err)
	}

	// Set our service with the new connection.
	s.conn = &conn

	log.Infof("Fims connection: %v", s.conn.Connected())
	return nil
}

// Close connection to the fims service
func (s *Service) Close() {
	if s.conn != nil {
		s.conn.Close()
	}
}

// Return the fims connection as an object.
func (s *Service) GetConnection() *fims.Fims {
	return s.conn
}
