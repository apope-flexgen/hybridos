// TODO: fix lint
/* eslint-disable max-lines */
import {
  Checkbox,
  Divider,
  Icon,
  IconList,
  Select,
  TMenuItem,
  TextField,
  Typography,
  ValidIconStrings,
} from '@flexgen/storybook';
import { SelectChangeEvent } from '@mui/material';
import { ChangeEvent } from 'react';
import { Asset } from 'shared/types/dtos/assets.dto';
import { useAssetsContext } from 'src/pages/UIConfig/TabContent/Assets';
import { SEPARATE_INPUTS_WITH_COMMA } from 'src/pages/UIConfig/TabContent/helpers/constants';
import {
  ADD_UNITS,
  ALARM_FIELDS,
  BASE_URI,
  ICON,
  ENABLE_SUMMARY,
  EXTENSION,
  FAULT_FIELDS,
  HAS_ALL_CONTROLS,
  HAS_SUMMARY,
  HOW_MANY_UNITS,
  ITEM_NAME,
  NAME,
  NAME_HELPER_TEXT,
  SOURCE_URI,
  RANGE,
  HAS_MAINTENANCE_ACTIONS,
} from './helpers/constants';
import { ColumnLeft, Row } from './styles';

const Items = () => {
  const { selectedAsset, setSelectedAsset } = useAssetsContext();

  const handleTextFieldChange = (event: ChangeEvent<HTMLInputElement | HTMLTextAreaElement>) => {
    const {
      target: { id, name, value },
    } = event;

    setSelectedAsset(
      (prevSelectedAsset) => ({
        ...prevSelectedAsset,
        info: {
          ...prevSelectedAsset?.info,
          [id || name]: value,
        },
      } as Asset),
    );
  };

  const handleSelectChangeEvent = (event: SelectChangeEvent, field: string) => {
    const {
      target: { value },
    } = event;

    setSelectedAsset(
      (prevSelectedAsset) => ({
        ...prevSelectedAsset,
        info: {
          ...prevSelectedAsset?.info,
          [field]: value,
        },
      } as Asset),
    );
  };
  const handleTextFieldCommaSeparatedChange = (
    event: ChangeEvent<HTMLInputElement | HTMLTextAreaElement>,
  ) => {
    const {
      target: { id, value },
    } = event;

    setSelectedAsset(
      (prevSelectedAsset) => ({
        ...prevSelectedAsset,
        info: {
          ...prevSelectedAsset?.info,
          [id]: value.split(',').map((v) => v.replace(' ', '')),
        },
      } as Asset),
    );
  };

  const handleCheckboxChange = (event: ChangeEvent<Element>, checked: boolean) => {
    const {
      target: { id },
    } = event;
    setSelectedAsset(
      (prevSelectedAsset) => ({
        ...prevSelectedAsset,
        info: {
          ...prevSelectedAsset?.info,
          [id]: checked,
        },
      } as Asset),
    );
  };

  console.log(selectedAsset?.info.icon);

  const iconMenuItems: TMenuItem[] = ValidIconStrings.map((icon) => ({
    name: icon,
    value: icon,
    component: <Icon src={icon as IconList} />,
  }));

  return (
    <>
      <Divider orientation="horizontal" variant="fullWidth" />
      <Row>
        <ColumnLeft>
          <Typography text={ITEM_NAME} variant="bodyL" />
        </ColumnLeft>
        <TextField
          color="primary"
          disableLabelAnimation
          id="itemName"
          label={ITEM_NAME}
          onChange={handleTextFieldChange}
          value={selectedAsset?.info.itemName || ''}
          inputProps={{ maxLength: 100 }}
        />
      </Row>
      <Divider orientation="horizontal" variant="fullWidth" />
      <Row>
        <ColumnLeft>
          <Typography text={NAME} variant="bodyL" />
        </ColumnLeft>
        <TextField
          color="primary"
          disableLabelAnimation
          helperText={NAME_HELPER_TEXT}
          id="name"
          label={NAME}
          onChange={handleTextFieldChange}
          value={selectedAsset?.info.name || ''}
          required
          inputProps={{ maxLength: 100 }}
        />
      </Row>
      <Divider orientation="horizontal" variant="fullWidth" />
      <Row>
        <ColumnLeft>
          <Typography text={ADD_UNITS} variant="bodyL" />
        </ColumnLeft>
        <TextField
          disableLabelAnimation
          id="numberOfItems"
          label={HOW_MANY_UNITS}
          onChange={handleTextFieldChange}
          value={selectedAsset?.info.numberOfItems || ''}
        />
      </Row>
      <Divider orientation="horizontal" variant="fullWidth" />
      <Row>
        <ColumnLeft>
          <Typography text={ICON} variant="bodyL" />
        </ColumnLeft>
        <Select
          minWidth={300}
          menuItems={iconMenuItems}
          id="icon"
          label={ICON}
          onChange={(e) => handleSelectChangeEvent(e, 'icon')}
          value={selectedAsset?.info.icon || ''}
        />
      </Row>
      <Divider orientation="horizontal" variant="fullWidth" />
      <Row>
        <ColumnLeft>
          <Typography text={ALARM_FIELDS} variant="bodyL" />
        </ColumnLeft>
        <TextField
          disableLabelAnimation
          helperText={SEPARATE_INPUTS_WITH_COMMA}
          id="alarmFields"
          label={ALARM_FIELDS}
          onChange={handleTextFieldCommaSeparatedChange}
          value={selectedAsset?.info.alarmFields?.join(',') || ''}
        />
      </Row>
      <Divider orientation="horizontal" variant="fullWidth" />
      <Row>
        <ColumnLeft>
          <Typography text={SOURCE_URI} variant="bodyL" />
        </ColumnLeft>
        <TextField
          disableLabelAnimation
          id="sourceURI"
          label={SOURCE_URI}
          onChange={handleTextFieldChange}
          value={selectedAsset?.info.sourceURI || ''}
          required
        />
      </Row>
      <Divider orientation="horizontal" variant="fullWidth" />
      <Row>
        <ColumnLeft>
          <Typography text={BASE_URI} variant="bodyL" />
        </ColumnLeft>
        <TextField
          disableLabelAnimation
          id="baseURI"
          label={BASE_URI}
          onChange={handleTextFieldChange}
          value={selectedAsset?.info.baseURI || ''}
        />
      </Row>
      <Divider orientation="horizontal" variant="fullWidth" />
      <Row>
        <ColumnLeft>
          <Typography text={EXTENSION} variant="bodyL" />
        </ColumnLeft>
        <TextField
          disableLabelAnimation
          id="extension"
          label={EXTENSION}
          onChange={handleTextFieldChange}
          value={selectedAsset?.info.extension || ''}
        />
      </Row>
      <Divider orientation="horizontal" variant="fullWidth" />
      <Row>
        <ColumnLeft>
          <Typography text={RANGE} variant="bodyL" />
        </ColumnLeft>
        <TextField
          disableLabelAnimation
          id="range"
          label={RANGE}
          onChange={handleTextFieldCommaSeparatedChange}
          value={selectedAsset?.info.range?.join(',') || ''}
        />
      </Row>
      <Divider orientation="horizontal" variant="fullWidth" />
      <Row>
        <ColumnLeft>
          <Typography text={FAULT_FIELDS} variant="bodyL" />
        </ColumnLeft>
        <TextField
          disableLabelAnimation
          helperText={SEPARATE_INPUTS_WITH_COMMA}
          id="faultFields"
          label={FAULT_FIELDS}
          onChange={handleTextFieldCommaSeparatedChange}
          value={selectedAsset?.info.faultFields?.join(',') || ''}
        />
      </Row>
      <Divider orientation="horizontal" variant="fullWidth" />
      <Row>
        <ColumnLeft>
          <Typography text={HAS_SUMMARY} variant="bodyL" />
        </ColumnLeft>
        <Checkbox
          color="primary"
          label={ENABLE_SUMMARY}
          id="hasSummary"
          onChange={handleCheckboxChange}
          value={!!selectedAsset?.info.hasSummary}
        />
      </Row>
      <Divider orientation="horizontal" variant="fullWidth" />
      <Row>
        <ColumnLeft>
          <Typography text={HAS_ALL_CONTROLS} variant="bodyL" />
        </ColumnLeft>
        <Checkbox
          color="primary"
          label={HAS_ALL_CONTROLS}
          id="hasBatchControls"
          onChange={handleCheckboxChange}
          value={!!selectedAsset?.info.hasBatchControls}
        />
      </Row>
      <Divider orientation="horizontal" variant="fullWidth" />
      <Row>
        <ColumnLeft>
          <Typography text={HAS_MAINTENANCE_ACTIONS} variant="bodyL" />
        </ColumnLeft>
        <Checkbox
          color="primary"
          label={HAS_MAINTENANCE_ACTIONS}
          id="hasMaintenanceActions"
          onChange={handleCheckboxChange}
          value={!!selectedAsset?.info.hasMaintenanceActions}
        />
      </Row>
    </>
  );
};

export default Items;
