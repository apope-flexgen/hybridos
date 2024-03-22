// TODO: fix lint
/* eslint-disable max-lines */
/* eslint-disable react/no-array-index-key */
import {
  Accordion,
  Divider,
  MuiButton,
  Switch,
  TextField,
  Typography,
} from '@flexgen/storybook';
import { ChangeEvent, useState } from 'react';
import { Dashboard } from 'shared/types/dtos/dashboards.dto';
import { useDashboardsContext } from 'src/pages/UIConfig/TabContent/Dashboard';
import {
  BATTERY_SOURCE_URI_HELPER,
  SEPARATE_INPUTS_WITH_COMMA,
} from 'src/pages/UIConfig/TabContent/helpers/constants';
import {
  ITEM_NAME,
  BATTERY_VIEW,
  ENABLE_BATTERY_VIEW,
  BATTERY_VIEW_SOURCE_URI,
  BATTERY_VIEW_URI,
  SOURCE_URI,
  BASE_URI,
  TEMPLATE,
  ENABLE_TEMPLATE,
  ITEMS,
  NO_ITEMS_YET,
  CREATE_NEW_ITEM,
  NAME,
  URI,
  DELETE_ITEM,
  ALARM_FIELDS,
  FAULT_FIELDS,
} from './helpers/constants';
import {
  AccordionDetailsSX, AccordionSX, ColumnLeft, Item, Row,
} from './styles';

const Items = () => {
  const { selectedDashboard, setSelectedDashboard } = useDashboardsContext();

  const [expanded, setExpanded] = useState(false);

  const handleTextFieldChange = (event: ChangeEvent<HTMLInputElement | HTMLTextAreaElement>) => {
    const {
      target: { id, name, value },
    } = event;

    setSelectedDashboard(
      (prevSelectedDashboard) => ({
        ...prevSelectedDashboard,
        info: {
          ...prevSelectedDashboard?.info,
          [id || name]: value,
        },
      } as Dashboard),
    );
  };

  const handleTextFieldCommaSeparatedChange = (
    event: ChangeEvent<HTMLInputElement | HTMLTextAreaElement>,
  ) => {
    const {
      target: { id, value },
    } = event;

    const valueWithoutSpaces = value.replace(/\s/g, '');
    const valueArray = valueWithoutSpaces.split(',');
    const arrayWithoutEmptyItems = valueArray.filter((item) => item !== '');

    setSelectedDashboard(
      (prevSelectedDashboard) => ({
        ...prevSelectedDashboard,
        info: {
          ...prevSelectedDashboard?.info,
          [id]: arrayWithoutEmptyItems,
        },
      } as Dashboard),
    );
  };

  const handleSwitchChange = (id: string, checked: boolean | undefined) => {
    setSelectedDashboard(
      (prevSelectedDashboard) => ({
        ...prevSelectedDashboard,
        info: {
          ...prevSelectedDashboard?.info,
          [id]: checked,
        },
      } as Dashboard),
    );
  };

  const handleNewItemClick = () => {
    setSelectedDashboard(
      (prevSelectedDashboard) => ({
        ...prevSelectedDashboard,
        info: {
          ...prevSelectedDashboard?.info,
          items: [
            ...(prevSelectedDashboard?.info.items || []),
            {
              name: '',
              uri: '',
            },
          ],
        },
      } as Dashboard),
    );
  };

  const handleItemChange = (
    event: ChangeEvent<HTMLInputElement | HTMLTextAreaElement>,
    index: number,
  ) => {
    const {
      target: { id, value },
    } = event;

    setSelectedDashboard((prevSelectedDashboard) => {
      const items = prevSelectedDashboard?.info.items?.map((item, i) => {
        if (i === index) {
          return {
            ...(prevSelectedDashboard.info.items?.[i] || {}),
            [id]: value,
          };
        }
        return item;
      });

      return {
        ...prevSelectedDashboard,
        info: {
          ...prevSelectedDashboard?.info,
          items,
        },
      } as Dashboard;
    });
  };

  const handleDeleteItemClick = (index: number) => {
    setSelectedDashboard(
      (prevSelectedDashboard) => ({
        ...prevSelectedDashboard,
        info: {
          ...prevSelectedDashboard?.info,
          items: (prevSelectedDashboard?.info.items || []).filter((item, i) => i !== index),
        },
      } as Dashboard),
    );
  };

  const handleExpand = () => {
    setExpanded((prevExpanded) => !prevExpanded);
  };

  return (
    <>
      <Row>
        <ColumnLeft>
          <Typography text={ITEM_NAME} variant="bodyL" />
        </ColumnLeft>
        <TextField
          color="primary"
          disableLabelAnimation
          id="name"
          label={ITEM_NAME}
          onChange={handleTextFieldChange}
          value={selectedDashboard?.info.name || ''}
          required
          inputProps={{ maxLength: 100 }}
        />
      </Row>
      <Divider orientation="horizontal" variant="fullWidth" />
      <Row>
        <ColumnLeft>
          <Typography text={SOURCE_URI} variant="bodyL" />
        </ColumnLeft>
        <TextField
          color="primary"
          disableLabelAnimation
          helperText={SEPARATE_INPUTS_WITH_COMMA}
          id="sourceURIs"
          label={SOURCE_URI}
          onChange={handleTextFieldCommaSeparatedChange}
          value={selectedDashboard?.info.sourceURIs?.toString() || ''}
        />
      </Row>
      <Divider orientation="horizontal" variant="fullWidth" />
      <Row>
        <ColumnLeft>
          <Typography text={BASE_URI} variant="bodyL" />
        </ColumnLeft>
        <TextField
          color="primary"
          disableLabelAnimation
          id="baseURI"
          label={BASE_URI}
          onChange={handleTextFieldChange}
          value={selectedDashboard?.info.baseURI || ''}
        />
      </Row>
      <Divider orientation="horizontal" variant="fullWidth" />
      <Row>
        <ColumnLeft>
          <Typography text={TEMPLATE} variant="bodyL" />
        </ColumnLeft>
        <Switch
          autoLayout
          color="primary"
          label={ENABLE_TEMPLATE}
          labelPlacement="right"
          onChange={(checked) => handleSwitchChange('isTemplate', checked)}
          value={selectedDashboard?.info.isTemplate}
        />
      </Row>
      {selectedDashboard?.info.isTemplate && (
        <>
          <Divider orientation="horizontal" variant="fullWidth" />
          <Row>
            <ColumnLeft>
              <Typography text={ITEMS} variant="bodyL" />
            </ColumnLeft>
            <Accordion
              accordionStyles={AccordionSX}
              accordionDetailsStyles={AccordionDetailsSX as React.CSSProperties}
              expanded={expanded}
              heading={selectedDashboard?.info.items?.length ? ITEMS : NO_ITEMS_YET}
              onChange={handleExpand}
            >
              <>
                {selectedDashboard?.info.items?.map((item, index) => (
                  <Item key={index}>
                    <TextField
                      color="primary"
                      disableLabelAnimation
                      id="name"
                      label={NAME}
                      onChange={(e) => handleItemChange(e, index)}
                      value={item.name}
                    />
                    <TextField
                      color="primary"
                      disableLabelAnimation
                      id="uri"
                      label={URI}
                      onChange={(e) => handleItemChange(e, index)}
                      value={item.uri}
                    />
                    <MuiButton
                      color="error"
                      label={DELETE_ITEM}
                      onClick={() => handleDeleteItemClick(index)}
                      variant="outlined"
                    />
                  </Item>
                ))}
                <MuiButton
                  label={CREATE_NEW_ITEM}
                  onClick={handleNewItemClick}
                  startIcon="Add"
                  variant="text"
                />
              </>
            </Accordion>
          </Row>
        </>
      )}
      {selectedDashboard?.info.isTemplate && (
        <>
          <Divider orientation="horizontal" variant="fullWidth" />
          <Row>
            <ColumnLeft>
              <Typography text={BATTERY_VIEW} variant="bodyL" />
            </ColumnLeft>
            <Switch
              autoLayout
              color="primary"
              label={ENABLE_BATTERY_VIEW}
              labelPlacement="right"
              onChange={(checked) => handleSwitchChange('batteryView', checked)}
              value={selectedDashboard?.info.batteryView}
            />
          </Row>
        </>
      )}
      {selectedDashboard?.info.batteryView && selectedDashboard?.info.isTemplate && (
        <Row>
          <ColumnLeft>
            <Typography text={BATTERY_VIEW_SOURCE_URI} variant="bodyM" />
          </ColumnLeft>
          <TextField
            color="primary"
            disableLabelAnimation
            helperText={BATTERY_SOURCE_URI_HELPER}
            id="batteryViewSourceURI"
            label={BATTERY_VIEW_SOURCE_URI}
            onChange={handleTextFieldChange}
            value={selectedDashboard?.info.batteryViewSourceURI || ''}
          />
        </Row>
      )}
      {selectedDashboard?.info.batteryView && selectedDashboard?.info.isTemplate && (
        <Row>
          <ColumnLeft>
            <Typography text={BATTERY_VIEW_URI} variant="bodyM" />
          </ColumnLeft>
          <TextField
            color="primary"
            disableLabelAnimation
            id="batteryViewURI"
            label={BATTERY_VIEW_URI}
            onChange={handleTextFieldChange}
            value={selectedDashboard?.info.batteryViewURI || ''}
          />
        </Row>
      )}
      <Divider orientation="horizontal" variant="fullWidth" />
      <Row>
        <ColumnLeft>
          <Typography text={ALARM_FIELDS} variant="bodyL" />
        </ColumnLeft>
        <TextField
          color="primary"
          disableLabelAnimation
          helperText={SEPARATE_INPUTS_WITH_COMMA}
          id="alarmFields"
          label={ALARM_FIELDS}
          onChange={handleTextFieldCommaSeparatedChange}
          value={selectedDashboard?.info.alarmFields?.toString() || ''}
        />
      </Row>
      <Divider orientation="horizontal" variant="fullWidth" />
      <Row>
        <ColumnLeft>
          <Typography text={FAULT_FIELDS} variant="bodyL" />
        </ColumnLeft>
        <TextField
          color="primary"
          disableLabelAnimation
          helperText={SEPARATE_INPUTS_WITH_COMMA}
          id="faultFields"
          label={FAULT_FIELDS}
          onChange={handleTextFieldCommaSeparatedChange}
          value={selectedDashboard?.info.faultFields?.toString() || ''}
        />
      </Row>
    </>
  );
};

export default Items;
