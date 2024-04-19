import {
  IconButton,
  Select,
  TextField,
} from '@flexgen/storybook';
import Box from '@flexgen/storybook/dist/components/Atoms/Box/Box';
import { templateRowBoxSx, replacementValueBoxSx, templateToFromBoxSx } from 'src/pages/ActivityLog/AlertManagement/alertManagement.styles';
import { Template } from 'src/pages/ActivityLog/activityLog.types';

export interface TemplateRowProps {
  allTemplates: Template[],
  template: Template,
  handleFieldChange: (field: string, updatedValue: any, commaSeparatedList?: boolean) => void,
  removeRow: (tempateId: string) => void;
}

// Input fields for a single template - wildcard, list, and value
const TemplateRow = ({
  allTemplates,
  template,
  handleFieldChange,
  removeRow,
}: TemplateRowProps) => {
  const handleTemplateFieldchange = (field: string, newValue: string | string[] | number) => {
    const templatesWithoutCurrent = allTemplates.filter(
      (existingTemplate) => template.id !== existingTemplate.id,
    );
    if (field) handleFieldChange('templates', [...templatesWithoutCurrent, { ...template, [field]: newValue }]);
  };

  return (
    <Box sx={templateRowBoxSx}>
      <TextField
        required
        label="Wildcard Character"
        value={template.token}
        onChange={(e) => handleTemplateFieldchange('token', e.target.value)}
      />
      <Select
        required
        minWidth={130}
        label="Template Type"
        menuItems={['list', 'sequential']}
        value={template.type}
        onChange={(e) => handleTemplateFieldchange('type', e.target.value)}
      />
      {
        template.type === 'sequential'
          ? (
            <Box sx={templateToFromBoxSx}>
              <TextField
                required
                fullWidth
                label="From"
                value={template.from?.toString() || ''}
                onChange={(e) => handleTemplateFieldchange('from', e.target.value)}
              />
              <TextField
                required
                fullWidth
                label="To"
                value={template.to?.toString() || ''}
                onChange={(e) => handleTemplateFieldchange('to', e.target.value)}
              />
            </Box>
          )
          : (
            <Box sx={replacementValueBoxSx}>
              <TextField
                required
                label="Replacement Values"
                value={template.type === 'list' ? (template.list || []).join(',') : `${template.from}..${template.to}`}
                onChange={(e) => handleTemplateFieldchange('list', e.target.value.split(',').map((v) => v.replace(' ', '')))}
                helperText="Enter a list of comma separated values to replace the wildcard character defined."
              />
            </Box>
          )
      }
      <IconButton icon="Close" onClick={() => removeRow(template.id)} />
    </Box>
  );
};

export default TemplateRow;
