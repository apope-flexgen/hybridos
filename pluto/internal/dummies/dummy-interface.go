// dummy-interface.go provides a fake, formatted json map of interface data.
package dummies

// Return a dummy network interface object for testing purposes.
func GetDummyNetworkInterface() map[string]interface{} {
	// Simulates network retrieval by returning a dummy data object.
	return map[string]interface{}{
		"interface":  "eth0",
		"attributes": "<BROADCAST,MULTICAST,UP,LOWER_UP>",
		"mtu":        1500,
		"qdisc":      "mq",
		"state":      "UP",
		"group":      "default",
		"qlen":       1000,
		"link": map[string]string{
			"ether": "00:15:5d:64:b8:05",
			"brd":   "ff:ff:ff:ff:ff:ff",
		},
		"inet": map[string]string{
			"address":       "172.26.42.210/20",
			"brd":           "172.26.47.255",
			"scope":         "global",
			"noprefixroute": "dynamic",
			"valid_lft":     "82402sec",
			"preferred_lft": "82402sec",
		},
		"inet6": map[string]string{
			"address":       "fe80::e0e3:3a36:bd4e:241a/64",
			"scope":         "link",
			"noprefixroute": "",
			"valid_lft":     "forever",
			"preferred_lft": "forever",
		},
	}
}
