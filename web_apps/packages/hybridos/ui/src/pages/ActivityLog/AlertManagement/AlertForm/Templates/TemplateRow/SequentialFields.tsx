import {
  Divider,
  NumericInput,

  Typography,
} from '@flexgen/storybook';
import Box from '@flexgen/storybook/dist/components/Atoms/Box/Box';
import { alertManagementHelperText } from 'src/pages/ActivityLog/AlertManagement/alertManagement.helpers';
import {
  templateRowTitleAndDescriptionSx,
  templateFieldsBoxSx,
  formRowSx,
} from 'src/pages/ActivityLog/AlertManagement/alertManagement.styles';
import { Template } from 'src/pages/ActivityLog/activityLog.types';

export interface SequentialFieldsProps {
  template: Template,
  handleTemplateFieldChange:(
    field: string,
    newValue: string | string[] | boolean | number,
  ) => void,
}

// Input fields for a single template - wildcard, list, and value
const SequentialFields = ({
  handleTemplateFieldChange,
  template,
}: SequentialFieldsProps) => (
  <>
    <Box sx={formRowSx}>
      <Box sx={templateRowTitleAndDescriptionSx}>
        <Typography text="From" variant="bodyL" />
      </Box>
      <Box sx={templateFieldsBoxSx}>
        <NumericInput
          validationRegEx="positiveIntegers"
          required
          label="From"
          value={template.from?.toString() || ''}
          onChange={(e) => handleTemplateFieldChange('from', e.target.value)}
        />
      </Box>
    </Box>
    <Divider variant="fullWidth" orientation="horizontal" />
    <Box sx={formRowSx}>
      <Box sx={templateRowTitleAndDescriptionSx}>
        <Typography text="To" variant="bodyL" />
      </Box>
      <Box sx={templateFieldsBoxSx}>
        <NumericInput
          validationRegEx="positiveIntegers"
          required
          label="To"
          value={template.to?.toString() || ''}
          onChange={(e) => handleTemplateFieldChange('to', e.target.value)}
        />
      </Box>
    </Box>
    <Divider variant="fullWidth" orientation="horizontal" />
    <Box sx={formRowSx}>
      <Box sx={templateRowTitleAndDescriptionSx}>
        <Typography text="Minimum Width" variant="bodyL" />
        <Typography text={alertManagementHelperText.padStart} variant="bodyS" />
        <Typography text={alertManagementHelperText.padStartExample1} variant="bodySBold" />
      </Box>
      <Box sx={templateFieldsBoxSx}>
        <NumericInput
          validationRegEx="positiveIntegers"
          value={template.minWidth?.toString() || '1'}
          onChange={(e) => handleTemplateFieldChange('minWidth', Number(e.target.value))}
        />
      </Box>
    </Box>
  </>
);

export default SequentialFields;
