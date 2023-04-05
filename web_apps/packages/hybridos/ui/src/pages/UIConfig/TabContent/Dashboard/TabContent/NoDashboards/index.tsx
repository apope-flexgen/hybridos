import {
  Box, CardRow, Label, MuiButton, Typography,
} from '@flexgen/storybook';
import { useDashboardsContext } from 'src/pages/UIConfig/TabContent/Dashboard';
import {
  CREATE_NEW_DASHBOARD_CARD, NO_DASHBOARD_CARDS_TO_DISPLAY, SETTING, VALUE,
} from './helpers/constants';
import BoxSX from './styles';

const NoDashboards = () => {
  const { handleAddDashboard } = useDashboardsContext();
  return (
    <>
      <CardRow>
        <Label className="setting" color="primary" size="medium" value={SETTING} />
        <Label color="primary" size="medium" value={VALUE} />
      </CardRow>
      <Box sx={BoxSX}>
        <Typography color="secondary" text={NO_DASHBOARD_CARDS_TO_DISPLAY} variant="helperText" />
        <MuiButton label={CREATE_NEW_DASHBOARD_CARD} onClick={handleAddDashboard} />
      </Box>
    </>
  );
};

export default NoDashboards;
