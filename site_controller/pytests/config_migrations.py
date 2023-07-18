from .fims import fims_set


# Stores config modifications that should be applied to site controller before launch, after launch, and after tests
class Config_Migrator:
    def __init__(self):
        keys = ["faults", "alarms"]
        sequence_names = ["Init", "Ready", "RunMode1", "RunMode2", "Shutdown", "Standby", "Startup"]
        seq_uris = [f"/dbi/site_controller/sequences/sequences/{name}/paths/0" for name in sequence_names]
        seq_migs = [{"uri": f"{uri}/active_{key}", "up": [{"name": f"/assets/get_any_ess_{key}"}], "down": [{"name": "/bypass"}]}
                    for uri in seq_uris for key in keys]
        self.alert_edits: list[dict] = [
            # Enable site level alarms and faults for all sequence types
            *seq_migs,
            # Modify register_ids to not compete with modbus_clients
            {
                "uri": "/dbi/site_controller/assets/assets/ess/asset_instances/0/components/0/variables/faults",
                "up": {"name": "Faults", "register_id": "test_faults", "type": "Int", "ui_type": "fault"},
                "down": {"name": "Faults", "register_id": "faults", "type": "Int", "ui_type": "fault"}
            },
            {
                "uri": "/dbi/site_controller/assets/assets/ess/asset_instances/0/components/0/variables/alarms",
                "up": {"name": "Alarms", "register_id": "test_alarms", "type": "Int", "ui_type": "alarm"},
                "down": {"name": "Alarms", "register_id": "alarms", "type": "Int", "ui_type": "alarm"}
            },
            {
                "uri": "/dbi/site_controller/sequences/sequences/RunMode1/paths/0/timeout",
                "up": {"value": 15},
                "down": {"value": 45}
            },
        ]
        
    def before_alerts(self):
        [fims_set(m["uri"], m["up"]) for m in self.alert_edits]
    def after_alerts(self):
        [fims_set(m["uri"], m["down"]) for m in self.alert_edits]
