// TODO: fix lint
/* eslint-disable max-lines */
import {
  Accordion,
  CardRow, Divider, Label, MuiButton, TextField,
} from '@flexgen/storybook';
import { ChangeEvent, useState } from 'react';
import { Asset, Summary as SummaryType } from 'shared/types/dtos/assets.dto';
import { useAssetsContext } from 'src/pages/UIConfig/TabContent/Assets';
import { ColumnTitles, TextFieldsContainer } from 'src/pages/UIConfig/TabContent/Assets/TabContent/styles';
import { AddItemButtonSX, DeleteButtonContainer } from 'src/pages/UIConfig/TabContent/styles';
import {
  ADD_SUMMARY, DELETE_SUMMARY, items, newSummary, SUMMARY,
} from './helpers/constants';

const Summary = () => {
  const {
    selectedAsset,
    setSelectedAsset,
  } = useAssetsContext();
  const [expanded, setExpanded] = useState(selectedAsset?.summary.length ? [0] : []);

  const handleAdd = () => {
    setSelectedAsset((prevSelectedAsset) => ({
      ...prevSelectedAsset,
      summary: [
        ...(prevSelectedAsset?.summary || []),
        newSummary,
      ],
    } as Asset));
    setExpanded((prevExpanded) => [...prevExpanded, selectedAsset?.summary.length || 0]);
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
      const summary = prevSelectedAsset?.summary.map((summ, i) => {
        if (i === index) {
          return {
            ...summ,
            [id]: value,
          };
        }
        return summ;
      });

      return {
        ...prevSelectedAsset,
        summary,
      } as Asset;
    });
  };

  const handleDelete = (index: number) => {
    setSelectedAsset((prevSelectedAsset) => {
      const summary = prevSelectedAsset?.summary.filter((_, i) => i !== index);
      return {
        ...prevSelectedAsset,
        summary,
      } as Asset;
    });
    setExpanded((prevExpanded) => prevExpanded.filter((i) => i !== index));
  };

  return (
    <>
      <CardRow alignItems="center">
        <ColumnTitles>
          <Label color="primary" size="medium" value={SUMMARY} />
        </ColumnTitles>
      </CardRow>
      <Divider orientation="horizontal" variant="fullWidth" />
      {selectedAsset?.summary.map((summ, index) => (
        <Accordion
          expanded={expanded.includes(index)}
          expandIcon={!expanded.includes(index) ? 'Edit' : undefined}
          heading={summ.name || ''}
          // eslint-disable-next-line react/no-array-index-key
          key={index}
          onChange={(exp) => handleExpand(index, exp)}
        >
          <TextFieldsContainer>
            {items.map(({
              key, label, helperText, type,
            }) => (
              <TextField
                disableLabelAnimation
                helperText={helperText}
                id={key}
                key={key}
                label={label}
                onChange={(e) => handleTextFieldChange(e, index)}
                value={summ[key as keyof SummaryType]}
                type={type as 'number' | undefined}
              />
            ))}
          </TextFieldsContainer>
          <DeleteButtonContainer>
            <MuiButton
              color="error"
              label={DELETE_SUMMARY}
              onClick={() => handleDelete(index)}
              variant="outlined"
            />
          </DeleteButtonContainer>
        </Accordion>
      ))}
      <MuiButton
        label={ADD_SUMMARY}
        onClick={handleAdd}
        size="small"
        startIcon="Add"
        sx={AddItemButtonSX}
        variant="outlined"
      />
    </>
  );
};

export default Summary;
