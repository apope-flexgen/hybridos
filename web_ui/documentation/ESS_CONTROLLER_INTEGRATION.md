In the web_ui config, include the keys "bms", "pcs", and "card_statuses".

"bms" and "pcs" are booleans, true to include them and false to hide them.

"card_statuses" is an object that holds arrays. Each key should be "type" and the value should be an array of up to 2 statuses to show. The statuses should be the "id" associated with the status.