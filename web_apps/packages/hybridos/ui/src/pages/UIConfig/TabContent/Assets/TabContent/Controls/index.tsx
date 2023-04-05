// TODO: fix lint
/* eslint-disable max-lines */
import {
  CardRow, Divider, Label, MuiButton, Select, TextField,
} from '@flexgen/storybook';
import { SelectChangeEvent } from '@mui/material';
import { ChangeEvent, useState, Fragment } from 'react';
import { Asset, Control } from 'shared/types/dtos/assets.dto';
import Accordion from 'src/components/Accordion';
import { useAssetsContext } from 'src/pages/UIConfig/TabContent/Assets';
import { ColumnTitles, TextFieldsContainer } from 'src/pages/UIConfig/TabContent/Assets/TabContent/styles';
import { AddItemButtonSX } from 'src/pages/UIConfig/TabContent/styles';
import {
  ADD_CONTROL, CONTROL, DELETE_CONTROL, items, newControl,
} from './helpers/constants';

const Controls = () => {
  const {
    selectedAsset,
    setSelectedAsset,
  } = useAssetsContext();
  const [expanded, setExpanded] = useState(selectedAsset?.controls.length ? [0] : []);

  const handleAdd = () => {
    setSelectedAsset((prevSelectedAsset) => ({
      ...prevSelectedAsset,
      controls: [
        ...(prevSelectedAsset?.controls || []),
        newControl,
      ],
    } as Asset));
    setExpanded((prevExpanded) => [...prevExpanded, selectedAsset?.controls.length || 0]);
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
      const controls = prevSelectedAsset?.controls.map((control, i) => {
        if (i === index) {
          return {
            ...control,
            [id || name]: value,
          };
        }
        return control;
      });

      return {
        ...prevSelectedAsset,
        controls,
      } as Asset;
    });
  };

  const handleDelete = (index: number) => {
    setSelectedAsset((prevSelectedAsset) => {
      const controls = prevSelectedAsset?.controls.filter((_, i) => i !== index);
      return {
        ...prevSelectedAsset,
        controls,
      } as Asset;
    });
    setExpanded((prevExpanded) => prevExpanded.filter((i) => i !== index));
  };

  const handleSelectChange = (e: SelectChangeEvent<string>, index: number) => {
    const { target: { value } } = e;

    setSelectedAsset((prevSelectedAsset) => {
      const controls = prevSelectedAsset?.controls.map((control, i) => {
        if (i === index) {
          return {
            ...control,
            inputType: value,
          };
        }
        return control;
      });
      return {
        ...prevSelectedAsset,
        controls,
      } as Asset;
    });
  };

  return (
    <>
      <CardRow alignItems="center">
        <ColumnTitles>
          <Label color="primary" size="medium" value={CONTROL} />
        </ColumnTitles>
      </CardRow>
      <Divider orientation="horizontal" variant="fullWidth" />
      {selectedAsset?.controls.map((control, index) => (
        <Accordion
          deleteText={DELETE_CONTROL}
          expanded={expanded.includes(index)}
          // eslint-disable-next-line react/no-array-index-key
          key={index}
          name={control.name || ''}
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
                    value={control[key as keyof Control]}
                  />
                ) : (
                  <TextField
                    disableLabelAnimation
                    helperText={helperText}
                    id={key}
                    key={key}
                    label={label}
                    onChange={(e) => handleTextFieldChange(e, index)}
                    value={control[key as keyof Control]}
                  />
                )}
              </Fragment>
            ))}
          </TextFieldsContainer>
        </Accordion>
      ))}
      <MuiButton
        label={ADD_CONTROL}
        onClick={handleAdd}
        size="small"
        startIcon="Add"
        sx={AddItemButtonSX}
        variant="outlined"
      />
    </>
  );
};

export default Controls;
