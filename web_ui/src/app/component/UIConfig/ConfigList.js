/* eslint-disable react/prop-types */
import React from 'react';

import {
    Paper,
    List,
    ListSubheader,
    ListItem,
    ListItemText,
} from '@mui/material';
import { withStyles } from 'tss-react/mui';
import { STYLES_CONFIG_LIST } from '../../styles';

import { capitalizeFirst } from '../../helpers';

import Loading from '../Loading';

class ConfigList extends React.Component {
    // TODO: Eventually add expand to list
    render() {
        const { selectedIndex, items, nameOfItem, handleItemSelect, retrievingItems, classes } = this.props;

        return (
            <Paper style={{ height: '65%', width: '100%', marginBottom: '10px', overflow: 'auto' }}>
                <List subheader={<li />}>
                    {/* <ListSubheader style={{ backgroundColor: 'primary', color: 'white' }}> */}
                    <ListSubheader className={classes.subheader}>
                        {capitalizeFirst(nameOfItem)} List
                    </ListSubheader>
                    {retrievingItems && <Loading size={50} styling={{ height: '35vh' }} />}
                    {(!retrievingItems && items) && items.map((item, index) => (
                        <ListItem
                            className = 'ui-list-item'
                            id = {(item.info.name.split(' ').join('')).toLowerCase()}
                            key={index}
                            onClick={() => handleItemSelect(item, index)}
                            selected={selectedIndex === index}
                            button
                        >
                            <ListItemText>{item.info.name}</ListItemText>
                        </ListItem>
                    ))}
                </List>
            </Paper>
        );
    }
}

export default withStyles(ConfigList, STYLES_CONFIG_LIST);
