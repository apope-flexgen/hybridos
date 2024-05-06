import {

  Divider,

  NumericInput,

  TextField,
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

export interface ListFieldsProps {
  template: Template,
  handleTemplateFieldChange:(
    field: string,
    newValue: string | string[] | boolean | number,
  ) => void,
  range: boolean,
}

// Input fields for a single template - wildcard, list, and value
const ListFields = ({
  handleTemplateFieldChange,
  template,
  range,
}: ListFieldsProps) => (
  <>
    <Box sx={formRowSx}>
      <Box sx={templateRowTitleAndDescriptionSx}>
        <Typography text="Replacement Values" variant="bodyL" />
        <Typography text={alertManagementHelperText.templateReplacementValues} variant="bodyS" />
        { range
      && (
      <>
        <Typography text={alertManagementHelperText.templateRangeReplacementValues} variant="bodyS" />
        <Typography text={alertManagementHelperText.templateRangeExample} variant="bodySBold" />
      </>
      )}
      </Box>
      <Box sx={templateFieldsBoxSx}>
        <TextField
          required
          label="Replacement Values"
          value={template.type === 'list' ? (template.list || []).join(',') : `${template.from}..${template.to}`}
          onChange={(e) => handleTemplateFieldChange('list', e.target.value.split(',').map((v) => v.replace(' ', '')))}
        />
      </Box>
    </Box>
    {
    range
    && (
      <>
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
    )
  }
  </>
);

export default ListFields;
