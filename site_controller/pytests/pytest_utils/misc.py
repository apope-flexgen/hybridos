from enum import Enum
from fims import fims_set

class ESS_LABELS(Enum):
    CONFIG_DEV = 0
    # do more if you want later

def set_bms_soc(value: int):
    for i in range(0, 9):
        uri = "/components/bms_1/sbmu_" + str(i) + "_soc_value"
        fims_set(uri, value, destination="psm")

def set_all_ess_soc(value: int, ess_config: ESS_LABELS):
    if ess_config.value == ESS_LABELS.CONFIG_DEV.value:
        fims_set("/components/ess_real_ls/bms_soc", value)
        fims_set("/components/ess_psm/bms_soc", value)

