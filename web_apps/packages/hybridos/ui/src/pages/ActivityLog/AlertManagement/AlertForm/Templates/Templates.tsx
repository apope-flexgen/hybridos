import {
  Typography, Divider, Switch, MuiButton,
} from '@flexgen/storybook';
import Box from '@flexgen/storybook/dist/components/Atoms/Box/Box';

import { useState, useEffect } from 'react';
import TemplateRow from 'src/pages/ActivityLog/AlertManagement/AlertForm/Templates/TemplateRow/TemplateRow';
import { useAlertFormContext } from 'src/pages/ActivityLog/AlertManagement/AlertForm/contexts/AlertFormContext';

import { generateInitialTemplates } from 'src/pages/ActivityLog/AlertManagement/alertManagement.helpers';
import {
  formRowSx,
  formRowTitleAndDescriptionSx,
  externalFormRowBoxSx,
  templateFieldsBoxSx,
  templateSwitchSx,
  accordionRowsSx,
} from 'src/pages/ActivityLog/AlertManagement/alertManagement.styles';
import { Template } from 'src/pages/ActivityLog/activityLog.types';

const Templates = () => {
  const { alertValues, handleFieldChange } = useAlertFormContext();

  const removeRow = (templateForRemoval: string) => {
    const newTemplates = alertValues.templates?.filter(
      (template: Template) => template.id !== templateForRemoval,
    );
    handleFieldChange('templates', newTemplates);
  };

  const initialTemplates = generateInitialTemplates(alertValues.templates || []);

  const addNewRow = () => handleFieldChange('templates', alertValues.templates ? [...alertValues.templates, ...initialTemplates] : initialTemplates);

  const generateTemplateRows = () => (
    alertValues.templates && alertValues.templates.length > 0
      ? alertValues.templates.sort((a, b) => a.id.toString().localeCompare(b.id))
      : []
  )
    .map((template) => (
      <TemplateRow
        removeRow={removeRow}
        template={template}
        handleFieldChange={handleFieldChange}
        allTemplates={alertValues.templates || []}
      />
    ));

  const [
    showTemplateFields,
    setShowTemplateFields,
  ] = useState<boolean>(alertValues.templates ? alertValues.templates.length > 0 : false);
  const [templateRows, setTemplateRows] = useState<JSX.Element[]>(generateTemplateRows());

  useEffect(() => {
    setTemplateRows(generateTemplateRows());
  }, [alertValues.templates]);

  const handleToggleTemplatesOff = () => {
    setShowTemplateFields(!showTemplateFields);
    handleFieldChange('templates', []);
  };

  return (
    <Box sx={externalFormRowBoxSx}>
      <Box sx={formRowSx}>
        <Box sx={formRowTitleAndDescriptionSx}>
          <Typography text="Templates" variant="bodyMBold" />
          <Typography text="Any templated wildcards to utilize in the aliases below" variant="bodyM" />
        </Box>
        <Box sx={templateFieldsBoxSx}>
          <Box sx={templateSwitchSx}>
            <Switch
              value={showTemplateFields}
              onChange={handleToggleTemplatesOff}
            />
            <Typography text="Configure templates" variant="bodySBold" />
          </Box>
          <Box sx={accordionRowsSx}>
            {
                showTemplateFields
                && templateRows
          }
          </Box>
          {
            showTemplateFields
            && <MuiButton variant="text" label="Add Template" startIcon="Add" onClick={addNewRow} />

          }
        </Box>
      </Box>
      <Divider variant="fullWidth" orientation="horizontal" />
    </Box>
  );
};

export default Templates;
