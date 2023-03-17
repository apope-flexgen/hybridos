/* eslint-disable react/prop-types */
import React from 'react';
import List from '@mui/material/List';
import ListItem from '@mui/material/ListItem';
import ListItemIcon from '@mui/material/ListItemIcon';
import ListItemText from '@mui/material/ListItemText';
import Collapse from '@mui/material/Collapse';
import Divider from '@mui/material/Divider';
import Typography from '@mui/material/Typography';

import ExpandLess from '@mui/icons-material/ExpandLess';
import ExpandMore from '@mui/icons-material/ExpandMore';
import WarningIcon from '@mui/icons-material/Warning';
import ErrorIcon from '@mui/icons-material/Error';
import FiberManualRecordIcon from '@mui/icons-material/FiberManualRecord';

/**
 * Renders alarms pane for Grid View
 */
class TabAlarms extends React.PureComponent {
    constructor() {
        super();
        this.state = { open: false };
    }

    /**
     * Handles opening of details for alarms
     */
    handleClick() {
        this.setState({ open: !this.state.open });
    }

    render() {
        const { open } = this.state;
        const { alarm } = this.props;

        return (
            <React.Fragment>
                <ListItem button onClick={() => this.handleClick()}>
                    <ListItemIcon>
                        {alarm.ui_type === 'alarm' ? <ErrorIcon style={{ fill: '#ffc107' }} /> : <WarningIcon color='secondary' />}
                    </ListItemIcon>
                    <ListItemText
                        primary={<Typography style={{ fontWeight: 'bold' }}>{alarm.name}</Typography>}
                        // secondary="November 11th, 2020 9:22AM"
                    />
                    {open ? <ExpandLess /> : <ExpandMore />}
                </ListItem>
                <Collapse in={open} timeout="auto" unmountOnExit>
                    <List component="div" disablePadding>
                        {alarm.options
                            && alarm.options.map((option, index) => (
                                <React.Fragment key={index}>
                                    <ListItem style={{ paddingLeft: '4em' }}>
                                        <ListItemIcon>
                                            <FiberManualRecordIcon />
                                        </ListItemIcon>
                                        <ListItemText
                                            primary={<Typography style={{ fontWeight: 'bold' }}>{option.name}</Typography>}
                                        />
                                    </ListItem>
                                </React.Fragment>
                            ))
                        }
                    </List>
                </Collapse>
                <Divider style={{ width: '100%' }}/>
            </React.Fragment>
        );
    }
}

export default TabAlarms;
