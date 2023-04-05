import { CardRow, Divider, Label } from '@flexgen/storybook';
import { ColumnTitles } from 'src/pages/UIConfig/TabContent/Dashboard/TabContent/styles';
import Items from './Items';
import { INFO, SET_VALUE } from './helpers/constants';

const Info = () => (
  <>
    <CardRow alignItems="center">
      <ColumnTitles>
        <Label color="primary" size="medium" value={INFO} />
        <Label color="primary" size="medium" value={SET_VALUE} />
      </ColumnTitles>
    </CardRow>
    <Divider orientation="horizontal" variant="fullWidth" />
    <Items />
  </>
);

export default Info;
