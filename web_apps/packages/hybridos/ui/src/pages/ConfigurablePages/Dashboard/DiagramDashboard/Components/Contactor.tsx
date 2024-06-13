import {
  Box, Icon, IconList, Typography,
} from '@flexgen/storybook';
import { contactorSx } from 'src/pages/ConfigurablePages/Dashboard/DiagramDashboard/diagramDashboard.styles';

interface ContactorProps {
  label: string;
  icon: IconList;
}

const Contactor = ({ label, icon }: ContactorProps) => (
  <Box sx={contactorSx}>
    <Icon src={icon} color="action" />
    <Typography text={label} variant="bodyS" />
  </Box>
);

export default Contactor;
