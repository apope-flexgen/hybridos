// TODO: fix lint
/* eslint-disable max-lines */
import {
  CardRow, Divider, Label, MuiButton, Select, TextField,
} from '@flexgen/storybook';
import { SelectChangeEvent } from '@mui/material';
import { ChangeEvent, Fragment, useState } from 'react';
import { AllControl, Asset } from 'shared/types/dtos/assets.dto';
import Accordion from 'src/components/Accordion';
import { useAssetsContext } from 'src/pages/UIConfig/TabContent/Assets';
import { ColumnTitles, TextFieldsContainer } from 'src/pages/UIConfig/TabContent/Assets/TabContent/styles';
import { AddItemButtonSX } from 'src/pages/UIConfig/TabContent/styles';
import {
  ADD_ALL_CONTROLS, ALL_CONTROLS, DELETE_ALL_CONTROLS, items, newAllControl,
} from './helpers/constants';

const AllControls = () => {
  const {
    selectedAsset,
    setSelectedAsset,
  } = useAssetsContext();
  const [expanded, setExpanded] = useState(selectedAsset?.allControls.length ? [0] : []);

  const handleAdd = () => {
    setSelectedAsset((prevSelectedAsset) => ({
      ...prevSelectedAsset,
      allControls: [
        ...(prevSelectedAsset?.allControls || []),
        newAllControl,
      ],
    } as Asset));
    setExpanded((prevExpanded) => [...prevExpanded, selectedAsset?.allControls.length || 0]);
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
      const allControls = prevSelectedAsset?.allControls.map((allControl, i) => {
        if (i === index) {
          return {
            ...allControl,
            [id || name]: value,
          };
        }
        return allControl;
      });

      return {
        ...prevSelectedAsset,
        allControls,
      } as Asset;
    });
  };

  const handleDelete = (index: number) => {
    setSelectedAsset((prevSelectedAsset) => {
      const allControls = prevSelectedAsset?.allControls.filter((_, i) => i !== index);
      return {
        ...prevSelectedAsset,
        allControls,
      } as Asset;
    });
    setExpanded((prevExpanded) => prevExpanded.filter((i) => i !== index));
  };
  const handleSelectChange = (e: SelectChangeEvent<string>, index: number) => {
    const { target: { value } } = e;

    setSelectedAsset((prevSelectedAsset) => {
      const allControls = prevSelectedAsset?.allControls.map((allControl, i) => {
        if (i === index) {
          return {
            ...allControl,
            inputType: value,
          };
        }
        return allControl;
      });
      return {
        ...prevSelectedAsset,
        allControls,
      } as Asset;
    });
  };

  return (
    <>
      <CardRow alignItems="center">
        <ColumnTitles>
          <Label color="primary" size="medium" value={ALL_CONTROLS} />
        </ColumnTitles>
      </CardRow>
      <Divider orientation="horizontal" variant="fullWidth" />
      {selectedAsset?.allControls.map((allControl, index) => (
        <Accordion
          deleteText={DELETE_ALL_CONTROLS}
          expanded={expanded.includes(index)}
          // eslint-disable-next-line react/no-array-index-key
          key={index}
          name={allControl.name || ''}
          onDelete={() => handleDelete(index)}
          onExpand={(_, exp) => handleExpand(index, exp)}
        >
          <TextFieldsContainer>
            {items.map(({
              key, label, helperText, select, options,
            }) => (
              select ? (
                <Select
                  label={label}
                  menuItems={options.map((option) => option.text)}
                  onChange={(e) => handleSelectChange(e, index)}
                  value={allControl[key as keyof AllControl]}
                />
              ) : (
                <TextField
                  disableLabelAnimation
                  helperText={helperText}
                  id={key}
                  key={key}
                  label={label}
                  onChange={(e) => handleTextFieldChange(e, index)}
                  value={allControl[key as keyof AllControl]}
                />
              )
            ))}
          </TextFieldsContainer>
        </Accordion>
      ))}
      <MuiButton
        label={ADD_ALL_CONTROLS}
        onClick={handleAdd}
        size="small"
        startIcon="Add"
        sx={AddItemButtonSX}
        variant="outlined"
      />
    </>
  );
};

export default AllControls;
