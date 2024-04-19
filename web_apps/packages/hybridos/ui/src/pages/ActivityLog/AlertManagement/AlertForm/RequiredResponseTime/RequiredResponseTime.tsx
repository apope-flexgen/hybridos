import {
  Typography, Select, Divider, NumericInput,
} from '@flexgen/storybook';
import Box from '@flexgen/storybook/dist/components/Atoms/Box/Box';

import { useAlertFormContext } from 'src/pages/ActivityLog/AlertManagement/AlertForm/contexts/AlertFormContext';

import {
  formRowSx, formRowTitleAndDescriptionSx, formRowContentsSx, externalFormRowBoxSx,
} from 'src/pages/ActivityLog/AlertManagement/alertManagement.styles';

// Input fields for the required resolution time of alert - value and unit
const RequiredResponseTime = () => {
  const { alertValues, handleFieldChange } = useAlertFormContext();

  return (
    <Box sx={externalFormRowBoxSx}>
      <Box sx={formRowSx}>
        <Box sx={formRowTitleAndDescriptionSx}>
          <Typography text="Required Resolution Time" variant="bodyMBold" />
          <Typography text="How quickly must this alert be resolved by personnel" variant="bodyM" />
        </Box>
        <Box sx={formRowContentsSx}>
          <NumericInput
            required
            validationRegEx="positiveIntegers"
            label="Resolution Time"
            value={alertValues.deadline.value.toString()}
            onChange={(e) => handleFieldChange('deadline', { ...alertValues.deadline, value: e.target.value })}
          />
          <Select
            menuItems={['minute(s)', 'hour(s)']}
            onChange={(e) => handleFieldChange('deadline', { ...alertValues.deadline, unit: e.target.value.replace('(s)', '') })}
            value={`${alertValues.deadline.unit}(s)`}
          />
        </Box>
      </Box>
      <Divider variant="fullWidth" orientation="horizontal" />
    </Box>
  );
};

export default RequiredResponseTime;
