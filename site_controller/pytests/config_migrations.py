from .fims import fims_set


# Stores config modifications that should be applied to site controller before launch, after launch, and after tests
class Config_Migrator:
    def upload(self, edits: 'list[dict]'):
        [fims_set(m["uri"], m["up"]) for m in edits]

    def download(self, edits: 'list[dict]'):
        [fims_set(m["uri"], m["down"]) for m in edits]
