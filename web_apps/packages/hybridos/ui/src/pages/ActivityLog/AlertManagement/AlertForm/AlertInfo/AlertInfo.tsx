import {
  Typography, TextField, Select, Divider, TMenuItem, Box,
} from '@flexgen/storybook';

import { useAlertFormContext } from 'src/pages/ActivityLog/AlertManagement/AlertForm/contexts/AlertFormContext';

import { alertManagementHelperText } from 'src/pages/ActivityLog/AlertManagement/alertManagement.helpers';
import {
  formRowSx,
  formRowTitleAndDescriptionSx,
  formRowContentsSx,
  externalFormRowBoxSx,
} from 'src/pages/ActivityLog/AlertManagement/alertManagement.styles';
import SeverityIndicator from 'src/pages/ActivityLog/Alerts/SeverityIndicator/SeverityIndicator';

// Input fields for title and severity of alert
const AlertInfo = () => {
  const { alertValues, handleFieldChange } = useAlertFormContext();

  const severityMenuItems: TMenuItem[] = [
    {
      name: 'Critical',
      value: 3,
      component: <SeverityIndicator severity={3} />,
    },
    {
      name: 'High',
      value: 2,
      component: <SeverityIndicator severity={2} />,
    },
    {
      name: 'Medium',
      value: 1,
      component: <SeverityIndicator severity={1} />,
    },
    {
      name: 'Low',
      value: 0,
      component: <SeverityIndicator severity={0} />,
    },
  ];

  return (
    <Box sx={externalFormRowBoxSx}>
      <Box sx={formRowSx}>
        <Box sx={formRowTitleAndDescriptionSx}>
          <Typography text="Alert Info" variant="bodyMBold" />
          <Typography text={alertManagementHelperText.alertInfo} variant="bodyM" />
        </Box>
        <Box sx={formRowContentsSx}>
          <TextField
            required
            label="Title"
            value={alertValues.title || ''}
            onChange={(e) => handleFieldChange('title', e.target.value)}
          />
          <Select
            required
            minWidth={200}
            label="Severity"
            menuItems={severityMenuItems}
            onChange={(e) => handleFieldChange('severity', e.target.value)}
            value={(alertValues.severity || '').toString()}
          />
        </Box>
      </Box>
      <Divider variant="fullWidth" orientation="horizontal" />
    </Box>
  );
};

export default AlertInfo;
