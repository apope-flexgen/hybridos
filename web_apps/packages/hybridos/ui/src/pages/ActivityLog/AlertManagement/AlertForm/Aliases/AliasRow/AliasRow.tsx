import {
  IconButton,
  Select,
  TextField,
} from '@flexgen/storybook';
import Box from '@flexgen/storybook/dist/components/Atoms/Box/Box';
import { templateRowBoxSx } from 'src/pages/ActivityLog/AlertManagement/alertManagement.styles';
import { Alias } from 'src/pages/ActivityLog/activityLog.types';

// Input fields for a single alias - alias, uri, and data type
export interface AliasRowProps {
  allAliases: Alias[],
  alias: Alias,
  handleFieldChange: (field: string, updatedValue: any, commaSeparatedList?: boolean) => void,
  removeRow: (aliasId: string) => void;
}
const AliasRow = ({
  allAliases,
  alias,
  handleFieldChange,
  removeRow,
}: AliasRowProps) => {
  const handleAliasFieldChange = (field: string, newValue: string) => {
    const aliasesWithoutCurrent = allAliases.filter(
      (existingAlias) => alias.id !== existingAlias.id,
    );
    if (field) handleFieldChange('aliases', [...aliasesWithoutCurrent, { ...alias, [field]: newValue }]);
  };
  return (
    <Box sx={templateRowBoxSx}>
      <TextField
        required
        label="Alias"
        value={alias.alias}
        onChange={(e) => handleAliasFieldChange('alias', e.target.value)}
      />
      <TextField
        required
        label="URI"
        value={alias.uri}
        color={(alias.uri && alias.uri.charAt(0) !== '/') ? 'error' : 'primary'}
        helperText="URIs should begin with the / character"
        onChange={(e) => handleAliasFieldChange('uri', e.target.value)}
      />
      <Select
        required
        menuItems={['number', 'boolean', 'string']}
        label="Data Type"
        value={alias.type}
        onChange={(e) => handleAliasFieldChange('type', e.target.value)}
      />
      <IconButton icon="Close" onClick={() => removeRow(alias.id)} />
    </Box>
  );
};
export default AliasRow;
