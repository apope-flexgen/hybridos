import { Typography, Divider } from '@flexgen/storybook';
import Box from '@flexgen/storybook/dist/components/Atoms/Box/Box';

import RuleBlock from 'src/pages/ActivityLog/AlertManagement/AlertForm/RuleLogic/RuleBlock/RuleBlock';
import { useAlertFormContext } from 'src/pages/ActivityLog/AlertManagement/AlertForm/contexts/AlertFormContext';

import { alertManagementHelperText } from 'src/pages/ActivityLog/AlertManagement/alertManagement.helpers';
import {
  formRowSx, formRowTitleAndDescriptionSx, formRowContentsSx, externalFormRowBoxSx,
} from 'src/pages/ActivityLog/AlertManagement/alertManagement.styles';

// Rule logic container, will contain the rule block with multiple conditions
const RuleLogic = () => {
  const { alertValues, handleFieldChange } = useAlertFormContext();

  return (
    <Box sx={externalFormRowBoxSx}>
      <Box sx={formRowSx}>
        <Box sx={formRowTitleAndDescriptionSx}>
          <Typography text="Rule Logic" variant="bodyMBold" />
          <Typography text={alertManagementHelperText.ruleLogic} variant="bodyM" />
        </Box>
        <Box sx={formRowContentsSx}>
          <RuleBlock
            conditions={alertValues.conditions}
            aliases={alertValues.aliases}
            handleFieldChange={handleFieldChange}
          />
        </Box>
      </Box>
      <Divider variant="fullWidth" orientation="horizontal" />
    </Box>
  );
};

export default RuleLogic;
