import { FunctionComponent } from 'react';
import { PasswordSettings } from 'shared/types/api/SiteAdmin.types';
import { siteAdminLabels } from 'src/pages/SiteAdmin/SiteAdmin.constants';

import SiteAdminTable from 'src/pages/SiteAdmin/SiteAdminTable';
import Multifactor from './Multifactor';
import OldPasswords from './OldPasswords';
import PasswordExpiration from './PasswordExpiration';
import PasswordLength from './PasswordLength';
import PasswordRequirements from './PasswordRequirements';

export interface PasswordSettingsProps {
  passwordSettings: PasswordSettings;
  setPasswordSettings: React.Dispatch<React.SetStateAction<PasswordSettings>>;
}
const PasswordSettingsFields: FunctionComponent<PasswordSettingsProps> = ({
  passwordSettings,
  setPasswordSettings,
}) => (
  <SiteAdminTable title={siteAdminLabels.localPageTitle}>
    <PasswordLength passwordSettings={passwordSettings} setPasswordSettings={setPasswordSettings} />
    <PasswordRequirements
      passwordSettings={passwordSettings}
      setPasswordSettings={setPasswordSettings}
    />
    <OldPasswords passwordSettings={passwordSettings} setPasswordSettings={setPasswordSettings} />
    <Multifactor passwordSettings={passwordSettings} setPasswordSettings={setPasswordSettings} />
    <PasswordExpiration
      passwordSettings={passwordSettings}
      setPasswordSettings={setPasswordSettings}
    />
  </SiteAdminTable>
);

export default PasswordSettingsFields;
