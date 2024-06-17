// serve.go instantiates our server listening node.
package fimsapi

import (
	"context"

	log "github.com/flexgen-power/go_flexgen/logger"
	fims "github.com/flexgen-power/hybridos/fims/go_fims"
)

// ListenAndServe starts the server instance and listens for messages.
func (s *Service) ListenAndServe(ctx context.Context) {
	log.Infof("Listening to FIMS messages...")

	// Listen to FIMS messages on a channel.
	fimsReceive := make(chan fims.FimsMsg)
	go s.conn.ReceiveChannel(fimsReceive)

	// Serve messages as they come in on the receive channel
	for {
		select {
		case <-ctx.Done():
			log.Infof("FIMS service shutting down")
			return
		case msg := <-fimsReceive:
			if err := s.Router.Serve(&msg); err != nil {
				log.Errorf("Serving message from %s with URI %s: %v", msg.ProcessName, msg.Uri, err)
			}
		}
	}
}
