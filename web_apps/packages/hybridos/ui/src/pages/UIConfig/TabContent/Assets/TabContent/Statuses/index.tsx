import {
  Accordion,
  CardRow, Divider, Label, MuiButton, TextField,
} from '@flexgen/storybook';
import { ChangeEvent, useState } from 'react';
import { Asset, Status } from 'shared/types/dtos/assets.dto';
import { useAssetsContext } from 'src/pages/UIConfig/TabContent/Assets';
import { ColumnTitles, TextFieldsContainer } from 'src/pages/UIConfig/TabContent/Assets/TabContent/styles';
import { AddItemButtonSX, DeleteButtonContainer } from 'src/pages/UIConfig/TabContent/styles';
import {
  ADD_STATUS, DELETE_STATUS, items, newStatus, STATUS,
} from './helpers/constants';

const Statuses = () => {
  const {
    selectedAsset,
    setSelectedAsset,
  } = useAssetsContext();
  const [expanded, setExpanded] = useState(selectedAsset?.statuses.length ? [0] : []);

  const handleAdd = () => {
    setSelectedAsset((prevSelectedAsset) => ({
      ...prevSelectedAsset,
      statuses: [
        ...(prevSelectedAsset?.statuses || []),
        newStatus,
      ],
    } as Asset));
    setExpanded((prevExpanded) => [...prevExpanded, selectedAsset?.statuses.length || 0]);
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
    const { target: { id, value } } = event;
    setSelectedAsset((prevSelectedAsset) => {
      const statuses = prevSelectedAsset?.statuses.map((status, i) => {
        if (i === index) {
          return {
            ...status,
            [id]: value,
          };
        }
        return status;
      });

      return {
        ...prevSelectedAsset,
        statuses,
      } as Asset;
    });
  };

  const handleDelete = (index: number) => {
    setSelectedAsset((prevSelectedAsset) => {
      const statuses = prevSelectedAsset?.statuses.filter((_, i) => i !== index);
      return {
        ...prevSelectedAsset,
        statuses,
      } as Asset;
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
      {selectedAsset?.statuses.map((status, index) => (
        <Accordion
          expanded={expanded.includes(index)}
          expandIcon={!expanded.includes(index) ? 'Edit' : undefined}
          heading={status.name || ''}
          // TODO: fix lint
          // eslint-disable-next-line react/no-array-index-key
          key={index}
          onChange={(exp) => handleExpand(index, exp)}
        >
          <TextFieldsContainer>
            {items.map((item) => (
              <TextField
                disableLabelAnimation
                helperText={item.helperText}
                id={item.key}
                key={item.key}
                label={item.label}
                onChange={(e) => handleTextFieldChange(e, index)}
                value={status[item.key as keyof Status]}
                type={item.type as 'number' | undefined}
              />
            ))}
          </TextFieldsContainer>
          <DeleteButtonContainer>
            <MuiButton
              color="error"
              label={DELETE_STATUS}
              onClick={() => handleDelete(index)}
              variant="outlined"
            />
          </DeleteButtonContainer>
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
