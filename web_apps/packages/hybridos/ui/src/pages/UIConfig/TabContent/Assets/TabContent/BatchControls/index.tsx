/* eslint-disable */
// TODO: fix lint
import {
  Accordion,
  CardRow,
  Divider,
  Label,
  MuiButton,
  Select,
  TextField,
} from '@flexgen/storybook';
import { SelectChangeEvent } from '@mui/material';
import { ChangeEvent, useState } from 'react';
import { Control, Asset } from 'shared/types/dtos/assets.dto';
import { useAssetsContext } from 'src/pages/UIConfig/TabContent/Assets';
import {
  ColumnTitles,
  TextFieldsContainer,
} from 'src/pages/UIConfig/TabContent/Assets/TabContent/styles';
import { AddItemButtonSX, DeleteButtonContainer } from 'src/pages/UIConfig/TabContent/styles';
import {
  ADD_BATCH_CONTROLS,
  BATCH_CONTROLS,
  DELETE_BATCH_CONTROLS,
  items,
  newBatchControl,
} from './helpers/constants';

const BatchControls = () => {
  const { selectedAsset, setSelectedAsset } = useAssetsContext();
  const [expanded, setExpanded] = useState(
    selectedAsset?.batchControls && selectedAsset?.batchControls.length ? [0] : [],
  );

  const handleAdd = () => {
    setSelectedAsset(
      (prevSelectedAsset) =>
        ({
          ...prevSelectedAsset,
          batchControls: [...(prevSelectedAsset?.batchControls || []), newBatchControl],
        } as Asset),
    );
    setExpanded((prevExpanded) => [...prevExpanded, selectedAsset?.batchControls?.length || 0]);
  };

  const handleExpand = (index: number, exp: boolean) => {
    if (exp) setExpanded((prevExpanded) => [...prevExpanded, index]);
    else {
      setExpanded((prevExpanded) =>
        prevExpanded.filter((expandedIndex) => expandedIndex !== index),
      );
    }
  };

  const handleTextFieldChange = (
    event: ChangeEvent<HTMLInputElement | HTMLTextAreaElement>,
    index: number,
  ) => {
    const {
      target: { id, value, name },
    } = event;
    setSelectedAsset((prevSelectedAsset) => {
      if (prevSelectedAsset?.batchControls === undefined) return prevSelectedAsset;

      const batchControls = prevSelectedAsset?.batchControls.map((batchControl, i) => {
        if (i === index) {
          return {
            ...batchControl,
            [id || name]: value,
          };
        }
        return batchControl;
      });

      return {
        ...prevSelectedAsset,
        batchControls,
      } as Asset;
    });
  };

  const handleDelete = (index: number) => {
    setSelectedAsset((prevSelectedAsset) => {
      if (prevSelectedAsset?.batchControls === undefined) return prevSelectedAsset;
      const batchControls = prevSelectedAsset?.batchControls.filter((_, i) => i !== index);
      return {
        ...prevSelectedAsset,
        batchControls,
      } as Asset;
    });
    setExpanded((prevExpanded) => prevExpanded.filter((i) => i !== index));
  };
  const handleSelectChange = (e: SelectChangeEvent<string>, index: number) => {
    const value = e.target.value

    setSelectedAsset((prevSelectedAsset) => {
      if (prevSelectedAsset?.batchControls === undefined) return prevSelectedAsset;

      const batchControls = prevSelectedAsset?.batchControls.map((batchControl, i) => {
        if (i === index) {
          return {
            ...batchControl,
            inputType: value,
          };
        }
        return batchControl;
      });
      return {
        ...prevSelectedAsset,
        batchControls,
      } as Asset;
    });
  };

  return (
    <>
      <CardRow alignItems='center'>
        <ColumnTitles>
          <Label color='primary' size='medium' value={BATCH_CONTROLS} />
        </ColumnTitles>
      </CardRow>
      <Divider orientation='horizontal' variant='fullWidth' />
      {selectedAsset?.batchControls &&
        selectedAsset?.batchControls.map((batchControl, index) => {
          return (
          <Accordion
            expanded={expanded.includes(index)}
            expandIcon={!expanded.includes(index) ? 'Edit' : undefined}
            heading={batchControl.name || ''}
            // TODO: fix lint
            // eslint-disable-next-line react/no-array-index-key
            key={index}
            onChange={(exp) => handleExpand(index, exp)}
          >
            <TextFieldsContainer>
              {items.map(({ key, label, helperText, select, options, type }) => {
                console.log(`${key}: ${batchControl[key]}`)
                 if (select) return (
                  <Select
                    label={label}
                    menuItems={options}
                    onChange={(e) => handleSelectChange(e, index)}
                    value={batchControl[key as keyof Control]}
                  />
                );
                else return (
                  <TextField
                    disableLabelAnimation
                    helperText={helperText}
                    id={key}
                    key={key}
                    label={label}
                    onChange={(e) => handleTextFieldChange(e, index)}
                    value={batchControl[key as keyof Control]}
                    type={type as 'number' | undefined}
                  />
                )
              }
              )}
            </TextFieldsContainer>
            <DeleteButtonContainer>
              <MuiButton
                color='error'
                label={DELETE_BATCH_CONTROLS}
                onClick={() => handleDelete(index)}
                variant='outlined'
              />
            </DeleteButtonContainer>
          </Accordion>
        )})}
      <MuiButton
        label={ADD_BATCH_CONTROLS}
        onClick={handleAdd}
        size='small'
        startIcon='Add'
        sx={AddItemButtonSX}
        variant='outlined'
      />
    </>
  );
};

export default BatchControls;
