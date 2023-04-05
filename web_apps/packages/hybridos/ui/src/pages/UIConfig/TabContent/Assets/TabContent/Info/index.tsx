import { CardRow, Divider, Label } from '@flexgen/storybook';
import { ColumnTitles } from 'src/pages/UIConfig/TabContent/Assets/TabContent/styles';
import Items from './Items';
import { INFO, SET_VALUE } from './helpers/constants';

const Info = () => (
  <>
    <CardRow alignItems="center" justifyContent="space-between">
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
