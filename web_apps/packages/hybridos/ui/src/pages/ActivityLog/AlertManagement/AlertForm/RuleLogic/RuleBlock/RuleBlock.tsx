import { MuiButton, ThemeType } from '@flexgen/storybook';
import Box from '@flexgen/storybook/dist/components/Atoms/Box/Box';

import { useState, useEffect } from 'react';
import ExpressionBlock from 'src/pages/ActivityLog/AlertManagement/AlertForm/RuleLogic/RuleBlock/ExpressionBlock/ExpressionBlock';
import { initialNewExpression } from 'src/pages/ActivityLog/AlertManagement/alertManagement.helpers';

import {
  ruleBlockBoxSx,
  setWidth,
} from 'src/pages/ActivityLog/AlertManagement/alertManagement.styles';

import { Expression, Alias } from 'src/pages/ActivityLog/activityLog.types';
import { useTheme } from 'styled-components';

export interface RuleBlockProps {
  conditions: Expression[];
  aliases: Alias[];
  handleFieldChange: (field: string, updatedValue: any, commaSeparatedList?: boolean) => void;
}
const RuleBlock = ({ conditions, aliases, handleFieldChange }: RuleBlockProps) => {
  const generateExpressionBlocks = (): JSX.Element | JSX.Element[] => {
    if (conditions.length > 0) {
      return conditions
        .sort((a, b) => a.index.toString().localeCompare(b.index.toString(), undefined, { numeric: true }))
        .map((expression) => (
          <ExpressionBlock
            expressions={conditions}
            aliases={aliases}
            expression={expression}
            handleFieldChange={handleFieldChange}
          />
        ));
    }
    return (
      <ExpressionBlock
        aliases={aliases}
        expressions={conditions}
        expression={null}
        handleFieldChange={handleFieldChange}
      />
    );
  };

  const [conditionRows, setConditionRows] = useState<JSX.Element | JSX.Element[]>(
    generateExpressionBlocks(),
  );

  useEffect(() => {
    setConditionRows(generateExpressionBlocks());
  }, [conditions, aliases]);

  const addNewCondition = () => {
    const newCondition = initialNewExpression(conditions);
    handleFieldChange('conditions', [...conditions, newCondition]);
  };

  const theme = useTheme() as ThemeType;

  return (
    <Box sx={ruleBlockBoxSx(theme)}>
      {conditionRows}
      <Box sx={setWidth(250)}>
        <MuiButton
          variant="text"
          label="Add Condition"
          startIcon="Add"
          onClick={addNewCondition}
          disabled={conditions.length === 0}
        />
      </Box>
    </Box>
  );
};
export default RuleBlock;
