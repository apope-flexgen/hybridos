// client.go implements callers for communicating to FIMS.
package fimsapi

import (
	"fmt"

	fims "github.com/flexgen-power/hybridos/fims/go_fims"
)

// Client structure to hold service reference
type Client struct {
	service *Service
}

// NewClient creates a new client instance.
func NewClient() *Client {
	return &Client{
		service: NewService(),
	}
}

// Close client connection to FIMS.
func (c *Client) Close() {
	c.service.Close()
}

// Make a connection to the FIMS service with given client.
func (c *Client) Connect(serviceName string) (err error) {
	// Connect to FIMS.
	*c.service.conn, err = fims.Connect(serviceName)
	if err != nil {
		return fmt.Errorf("failed to connect with service %v to FIMS: %w", serviceName, err)
	}

	return nil
}

// SendMessage sets up the connection, sends a message, and closes the connection
func (c *Client) Send(body interface{}, msg *fims.FimsMsg) (err error) {

	// Send message to FIMS.
	if err := c.service.conn.SendSet(msg.Replyto, "", body); err != nil {
		return fmt.Errorf("sending message to %s: %v", msg.Replyto, err)
	}

	return nil
}

// Sets up a client and send a message to FIMS, then closes connection.
// msg in parameter list is the message recieved from FIMS, and
// msg.Replyto is the address to send the message back to.
func SendMessage(serviceName string, body interface{}, msg *fims.FimsMsg) (err error) {
	client := NewClient()
	defer client.Close()

	// Connect to FIMS.
	if err := client.Connect(serviceName); err != nil {
		return fmt.Errorf("connecting to FIMS: %v", err)
	}

	// Send message to FIMS.
	if err := client.Send(body, msg); err != nil {
		return fmt.Errorf("sending message to FIMS: %v", err)
	}

	return nil
}
