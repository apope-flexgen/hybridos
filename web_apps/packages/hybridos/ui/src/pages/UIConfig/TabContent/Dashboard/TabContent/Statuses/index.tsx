// TODO: fix lint
/* eslint-disable react/no-array-index-key */
import {
  CardRow, Divider, Label, MuiButton, TextField,
} from '@flexgen/storybook';
import { ChangeEvent, useState } from 'react';
import { Dashboard, Status } from 'shared/types/dtos/dashboards.dto';
import Accordion from 'src/components/Accordion';
import { useDashboardsContext } from 'src/pages/UIConfig/TabContent/Dashboard';
import { ColumnTitles, TextFieldsContainer } from 'src/pages/UIConfig/TabContent/Dashboard/TabContent/styles';
import { AddItemButtonSX } from 'src/pages/UIConfig/TabContent/styles';
import {
  ADD_STATUS, DELETE_STATUS, items, newStatus, STATUS,
} from './helpers/constants';

const Statuses = () => {
  const {
    selectedDashboard,
    setSelectedDashboard,
  } = useDashboardsContext();
  const [expanded, setExpanded] = useState(selectedDashboard?.status.length ? [0] : []);

  const handleAdd = () => {
    setSelectedDashboard((prevSelectedDashboard) => ({
      ...prevSelectedDashboard,
      status: [
        ...(prevSelectedDashboard?.status || []),
        newStatus,
      ],
    } as Dashboard));
    setExpanded((prevExpanded) => [...prevExpanded, selectedDashboard?.status.length || 0]);
  };

  const handleTextFieldChange = (
    event: ChangeEvent<HTMLInputElement | HTMLTextAreaElement>,
    index: number,
  ) => {
    const { target: { id, value } } = event;
    setSelectedDashboard((prevSelectedDashboard) => {
      const status = prevSelectedDashboard?.status.map((st, i) => {
        if (i === index) {
          return {
            ...st,
            [id]: value,
          };
        }
        return st;
      });

      return {
        ...prevSelectedDashboard,
        status,
      } as Dashboard;
    });
  };

  const handleExpand = (index: number, exp: boolean) => {
    if (exp) setExpanded((prevExpanded) => [...prevExpanded, index]);
    else {
      setExpanded(
        (prevExpanded) => prevExpanded.filter(
          (expandedIndex) => expandedIndex !== index,
        ),
      );
    }
  };

  const handleDelete = (index: number) => {
    setSelectedDashboard((prevSelectedDashboard) => {
      const status = prevSelectedDashboard?.status.filter((_, i) => i !== index);
      return {
        ...prevSelectedDashboard,
        status,
      } as Dashboard;
    });
    setExpanded((prevExpanded) => prevExpanded.filter((i) => i !== index));
  };

  return (
    <>
      <CardRow alignItems="center">
        <ColumnTitles>
          <Label color="primary" size="medium" value={STATUS} />
        </ColumnTitles>
      </CardRow>
      <Divider orientation="horizontal" variant="fullWidth" />
      {selectedDashboard?.status.map((st, index) => (
        <Accordion
          deleteText={DELETE_STATUS}
          expanded={expanded.includes(index)}
          key={index}
          name={st.name || ''}
          onDelete={() => handleDelete(index)}
          onExpand={(_, exp) => handleExpand(index, exp)}
        >
          <TextFieldsContainer>
            {items.map((item, i) => (
              <TextField
                disableLabelAnimation
                helperText={item.helperText}
                id={item.key}
                key={i}
                label={item.label}
                onChange={(e) => handleTextFieldChange(e, index)}
                value={st[item.key as keyof Status]}
              />
            ))}
          </TextFieldsContainer>
        </Accordion>
      ))}
      <MuiButton
        label={ADD_STATUS}
        onClick={handleAdd}
        size="small"
        startIcon="Add"
        sx={AddItemButtonSX}
        variant="outlined"
      />
    </>
  );
};

export default Statuses;
