// TODO: fix lint
/* eslint-disable max-lines */
import {
  CardRow, Divider, Label, MuiButton, Select, TextField,
} from '@flexgen/storybook';
import { SelectChangeEvent } from '@mui/material';
import { ChangeEvent, Fragment, useState } from 'react';
import { Asset, SummaryControl } from 'shared/types/dtos/assets.dto';
import Accordion from 'src/components/Accordion';
import { useAssetsContext } from 'src/pages/UIConfig/TabContent/Assets';
import { ColumnTitles, TextFieldsContainer } from 'src/pages/UIConfig/TabContent/Assets/TabContent/styles';
import { AddItemButtonSX } from 'src/pages/UIConfig/TabContent/styles';
import {
  ADD_SUMMARY_CONTROL, DELETE_SUMMARY_CONTROL, items, newSummaryControl, SUMMARY_CONTROLS,
} from './helpers/constants';

const SummaryControls = () => {
  const {
    selectedAsset,
    setSelectedAsset,
  } = useAssetsContext();
  const [expanded, setExpanded] = useState(selectedAsset?.summaryControls.length ? [0] : []);

  const handleAdd = () => {
    setSelectedAsset((prevSelectedAsset) => ({
      ...prevSelectedAsset,
      summaryControls: [
        ...(prevSelectedAsset?.summaryControls || []),
        newSummaryControl,
      ],
    } as Asset));
    setExpanded((prevExpanded) => [...prevExpanded, selectedAsset?.summaryControls.length || 0]);
  };

  const handleExpand = (index: number, exp: boolean) => {
    if (exp) setExpanded((prevExpanded) => [...prevExpanded, index]);
    else {
      setExpanded(
        (prevExpanded) => prevExpanded.filter((expandedIndex) => expandedIndex !== index),
      );
    }
  };

  const handleTextFieldChange = (
    event: ChangeEvent<HTMLInputElement | HTMLTextAreaElement>,
    index: number,
  ) => {
    const { target: { id, value, name } } = event;
    setSelectedAsset((prevSelectedAsset) => {
      const summaryControls = prevSelectedAsset?.summaryControls.map((summaryControl, i) => {
        if (i === index) {
          return {
            ...summaryControl,
            [id || name]: value,
          };
        }
        return summaryControl;
      });

      return {
        ...prevSelectedAsset,
        summaryControls,
      } as Asset;
    });
  };

  const handleDelete = (index: number) => {
    setSelectedAsset((prevSelectedAsset) => {
      const summaryControls = prevSelectedAsset?.summaryControls.filter((_, i) => i !== index);
      return {
        ...prevSelectedAsset,
        summaryControls,
      } as Asset;
    });
    setExpanded((prevExpanded) => prevExpanded.filter((i) => i !== index));
  };

  const handleSelectChange = (e: SelectChangeEvent<string>, index: number) => {
    const { target: { value } } = e;

    setSelectedAsset((prevSelectedAsset) => {
      const summaryControls = prevSelectedAsset?.summaryControls.map((summaryControl, i) => {
        if (i === index) {
          return {
            ...summaryControl,
            inputType: value,
          };
        }
        return summaryControl;
      });
      return {
        ...prevSelectedAsset,
        summaryControls,
      } as Asset;
    });
  };

  return (
    <>
      <CardRow alignItems="center">
        <ColumnTitles>
          <Label color="primary" size="medium" value={SUMMARY_CONTROLS} />
        </ColumnTitles>
      </CardRow>
      <Divider orientation="horizontal" variant="fullWidth" />
      {selectedAsset?.summaryControls.map((summaryControl, index) => (
        <Accordion
          deleteText={DELETE_SUMMARY_CONTROL}
          expanded={expanded.includes(index)}
          // eslint-disable-next-line react/no-array-index-key
          key={index}
          name={summaryControl.name || ''}
          onDelete={() => handleDelete(index)}
          onExpand={(_, exp) => handleExpand(index, exp)}
        >
          <TextFieldsContainer>
            {items.map(({
              key, label, helperText, select, options,
            }) => (
              <Fragment key={key}>
                {select ? (
                  <Select
                    label={label}
                    menuItems={options.map((option) => option.text)}
                    onChange={(e) => handleSelectChange(e, index)}
                    value={summaryControl[key as keyof SummaryControl]}
                  />
                ) : (
                  <TextField
                    disableLabelAnimation
                    helperText={helperText}
                    id={key}
                    key={key}
                    label={label}
                    onChange={(e) => handleTextFieldChange(e, index)}
                    value={summaryControl[key as keyof SummaryControl]}
                  />
                )}
              </Fragment>
            ))}
          </TextFieldsContainer>
        </Accordion>
      ))}
      <MuiButton
        label={ADD_SUMMARY_CONTROL}
        onClick={handleAdd}
        size="small"
        startIcon="Add"
        sx={AddItemButtonSX}
        variant="outlined"
      />
    </>
  );
};

export default SummaryControls;
