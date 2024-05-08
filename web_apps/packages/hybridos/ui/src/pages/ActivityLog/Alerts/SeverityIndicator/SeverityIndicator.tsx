import { Box, ColorType, Icon } from '@flexgen/storybook';

export interface SeverityIndicatorProps {
  severity: number;
}

const SeverityIndicator = ({ severity }: SeverityIndicatorProps) => {
  const numArray = [0, 1, 2, 3];
  const colorMapping: { [key: number]: ColorType | 'inherit' } = {
    0: 'inherit',
    1: 'info',
    2: 'warning',
    3: 'error',
  };
  return (
    <Box>
      {numArray.map((number) => (
        <Icon
          src="Circle"
          size={number <= severity ? '8px' : '4px'}
          color={number <= severity ? colorMapping[severity] : 'disabled'}
        />
      ))}
    </Box>
  );
};

export default SeverityIndicator;
