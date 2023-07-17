package websockets

import "fmt"

type ClientMap map[string]*client

var Clients ClientMap = ClientMap{}

func disconnectAllClients() {
	for _, client := range Clients {
		client.disconnect()
	}
	Clients = make(ClientMap)
}

// Launches all clients in separate goroutines.
func (clientMap ClientMap) LaunchAll() {
	for _, client := range clientMap {
		go client.launch()
	}
}

// Adds the given client to the map.
func (clientMap ClientMap) Add(newClient *client) {
	clientMap[newClient.id] = newClient
}

// Searches the map for a client with the given string.
// Returns the client if found, or an error if not found.
func (clientMap ClientMap) Get(id string) (*client, error) {
	client, ok := clientMap[id]
	if !ok {
		return nil, fmt.Errorf("could not find client with ID %s", id)
	}
	return client, nil
}

// Returns true if the client is connected. Returns false if the client
// is not connected or no client was found with the given ID.
func (clientMap ClientMap) ClientIsConnected(id string) bool {
	client, ok := clientMap[id]
	if !ok {
		return false
	}

	return client.IsConnected()
}

// Writes the given message to the WebSocket connection for the client.
func (clientMap ClientMap) WriteTo(id string, msg Msg) error {
	client, ok := clientMap[id]
	if !ok {
		return fmt.Errorf("could not find %s in client map", id)
	}

	if err := client.WriteJSON(msg); err != nil {
		return fmt.Errorf("failed to write to client connection: %w", err)
	}
	return nil
}
