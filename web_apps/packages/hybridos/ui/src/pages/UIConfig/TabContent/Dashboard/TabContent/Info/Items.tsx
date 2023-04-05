// TODO: fix lint
/* eslint-disable max-lines */
/* eslint-disable react/no-array-index-key */
import {
  Divider, MuiButton, Switch, TextField, Typography,
} from '@flexgen/storybook';
import { Accordion, AccordionSummary, AccordionDetails } from '@mui/material';
import { ChangeEvent } from 'react';
import { Dashboard } from 'shared/types/dtos/dashboards.dto';
import { useDashboardsContext } from 'src/pages/UIConfig/TabContent/Dashboard';
import { SEPARATE_INPUTS_WITH_COMMA } from 'src/pages/UIConfig/TabContent/helpers/constants';
import {
  ITEM_NAME,
  BATTERY_VIEW,
  ENABLE_BATTERY_VIEW,
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
} from './helpers/constants';
import {
  AccordionDetailsSX, AccordionSummarySX, AccordionSX, ColumnLeft, Item, Row,
} from './styles';

const Items = () => {
  const {
    selectedDashboard,
    setSelectedDashboard,
  } = useDashboardsContext();

  const handleTextFieldChange = (event: ChangeEvent<HTMLInputElement | HTMLTextAreaElement>) => {
    const { target: { id, name, value } } = event;

    setSelectedDashboard((prevSelectedDashboard) => ({
      ...prevSelectedDashboard,
      info: {
        ...prevSelectedDashboard?.info,
        [id || name]: value,
      },
    } as Dashboard));
  };

  const handleTextFieldCommaSeparatedChange = (
    event: ChangeEvent<HTMLInputElement | HTMLTextAreaElement>,
  ) => {
    const { target: { id, value } } = event;

    setSelectedDashboard((prevSelectedDashboard) => ({
      ...prevSelectedDashboard,
      info: {
        ...prevSelectedDashboard?.info,
        [id]: value,
      },
    } as Dashboard));
  };

  const handleSwitchChange = (id: string, checked: boolean | undefined) => {
    setSelectedDashboard((prevSelectedDashboard) => ({
      ...prevSelectedDashboard,
      info: {
        ...prevSelectedDashboard?.info,
        [id]: checked,
      },
    } as Dashboard));
  };

  const handleNewItemClick = () => {
    setSelectedDashboard((prevSelectedDashboard) => ({
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
    } as Dashboard));
  };

  const handleItemChange = (
    event: ChangeEvent<HTMLInputElement | HTMLTextAreaElement>,
    index: number,
  ) => {
    const { target: { id, value } } = event;

    setSelectedDashboard((prevSelectedDashboard) => {
      const items = prevSelectedDashboard?.info.items?.map((item, i) => {
        if (i === index) {
          return ({
            ...(prevSelectedDashboard.info.items?.[i] || {}),
            [id]: value,
          });
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
    setSelectedDashboard((prevSelectedDashboard) => ({
      ...prevSelectedDashboard,
      info: {
        ...prevSelectedDashboard?.info,
        items: (prevSelectedDashboard?.info.items || []).filter((item, i) => i !== index),
      },
    } as Dashboard));
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
        />
      </Row>
      <Divider orientation="horizontal" variant="fullWidth" />
      <Row>
        <ColumnLeft>
          <Typography text={BATTERY_VIEW} variant="bodyL" />
        </ColumnLeft>
        <Switch
          color="primary"
          label={ENABLE_BATTERY_VIEW}
          labelPlacement="right"
          onChange={(checked) => handleSwitchChange('batteryView', checked)}
          value={selectedDashboard?.info.batteryView}
        />
      </Row>
      <Divider orientation="horizontal" variant="fullWidth" />
      <Row>
        <ColumnLeft>
          <Typography text={TEMPLATE} variant="bodyL" />
        </ColumnLeft>
        <Switch
          color="primary"
          label={ENABLE_TEMPLATE}
          labelPlacement="right"
          onChange={(checked) => handleSwitchChange('isTemplate', checked)}
          value={selectedDashboard?.info.isTemplate}
        />
      </Row>
      <Divider orientation="horizontal" variant="fullWidth" />
      <Row>
        <ColumnLeft>
          <Typography text={ITEMS} variant="bodyL" />
        </ColumnLeft>
        <Accordion sx={AccordionSX}>
          <AccordionSummary
            sx={AccordionSummarySX}
          >
            <Typography
              color="disabled"
              text={selectedDashboard?.info.items?.length ? ITEMS : NO_ITEMS_YET}
              variant="bodyMBold"
            />
          </AccordionSummary>
          <AccordionDetails sx={AccordionDetailsSX}>
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
          </AccordionDetails>
        </Accordion>
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
          id="baseURI"
          label={BASE_URI}
          onChange={handleTextFieldCommaSeparatedChange}
          value={selectedDashboard?.info.baseURI || ''}
        />
      </Row>
      <Divider orientation="horizontal" variant="fullWidth" />
    </>
  );
};

export default Items;
