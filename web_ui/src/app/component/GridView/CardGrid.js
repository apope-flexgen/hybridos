/* eslint-disable react/prop-types */
/* eslint-disable camelcase */
import React from 'react';
import Box from '@mui/material/Box';
import Grid from '@mui/material/Grid';

import GridCard from './GridCard';

/**
 * Renders card grid
 * @param {*} props
 */
function CardGridRender(props) {
    const {
        asset_components,
        parentClickHandler,
        updateAssetPage,
        assetType,
        selected_object,
    } = props;

    let size;
    if (asset_components.length < 16) size = 3;
    else if (asset_components.length > 24) size = 6;
    else size = 4;

    const itemSize = 12 / size;

    const grid = [];
    let componentRow = [];
    for (let i = 0; i < asset_components.length; i += 1) {
        if (i !== 0 && i % size === 0) {
            grid.push(componentRow);
            componentRow = [];
        }
        componentRow.push(asset_components[i]);
    }
    if (componentRow.length !== 0) grid.push(componentRow);

    const theGrid = grid.map((row, index) => (
        <FormRow
            key={index}
            row={row}
            parentClickHandler={parentClickHandler}
            updateAssetPage={updateAssetPage}
            assetType={assetType}
            selected_object={selected_object}
            itemSize={itemSize}
            size={size}
        />
    ));
    return (
        <>
            {theGrid}
        </>
    );
}

/**
 * Renders row for card grid
 * @param {*} props
 */
function FormRow(props) {
    const {
        row,
        parentClickHandler,
        updateAssetPage,
        assetType,
        selected_object,
        itemSize,
        size,
    } = props;

    const theRow = row.map((component, index) => (
        <Grid item xs={itemSize} key={index}>
            <GridCard
                parentClickHandler={parentClickHandler}
                component={component}
                asset={component}
                updateAssetPage={updateAssetPage}
                assetType={assetType}
                selected_object={selected_object}
                size={size}
            />
        </Grid>
    ));

    return (
        <>
            {theRow}
        </>
    );
}

/**
 * Creates container for card grid
 */
class CardGrid extends React.PureComponent {
    render() {
        const {
            asset_components,
            parentClickHandler,
            updateAssetPage,
            assetType,
            selected_object,
        } = this.props;

        return (
            <Box flexGrow={1} style={{ marginLeft: '18px' }}>
                <Grid container spacing={1}>
                    {asset_components && <CardGridRender
                        asset_components={asset_components}
                        parentClickHandler={parentClickHandler}
                        updateAssetPage={updateAssetPage}
                        assetType={assetType}
                        selected_object={selected_object}
                    />}
                </Grid>
            </Box>
        );
    }
}

export default CardGrid;
