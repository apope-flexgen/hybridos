import {
  CardRow, Label, MuiButton, EmptyContainer,
} from '@flexgen/storybook';
import { useDashboardsContext } from 'src/pages/UIConfig/TabContent/Dashboard';
import {
  CREATE_NEW_DASHBOARD_CARD,
  NO_DASHBOARD_CARDS_TO_DISPLAY,
  SETTING,
  VALUE,
} from './helpers/constants';

const NoDashboards = () => {
  const { handleAddDashboard } = useDashboardsContext();
  return (
    <>
      <CardRow>
        <Label className="setting" color="primary" size="medium" value={SETTING} />
        <Label color="primary" size="medium" value={VALUE} />
      </CardRow>
      <EmptyContainer text={NO_DASHBOARD_CARDS_TO_DISPLAY}>
        <MuiButton label={CREATE_NEW_DASHBOARD_CARD} onClick={() => handleAddDashboard()} />
      </EmptyContainer>
    </>
  );
};

export default NoDashboards;
