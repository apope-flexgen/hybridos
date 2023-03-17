/* eslint-disable react/prop-types */
import React from 'react';

import {
    Paper,
    AppBar,
    Tab,
    Tabs,
    Box,
    Typography,
} from '@mui/material';

import ConfigTab from './ConfigTab';
import Loading from '../Loading';

let  initialTab = false;

function TabPanel(props) {
    const { children, selectedTab, index } = props;

    return (
        <React.Fragment>
            {selectedTab === index && (
                <Box overflow="auto" p={3} style={{ maxHeight: '67vh' }}>
                    <Typography>{children}</Typography>
                </Box>
            )}
        </React.Fragment>
    );
}

class ConfigTabs extends React.Component {
    constructor() {
        super();
        this.state = {
            selectedTab: 0,
        };
    }

    handleTab = (value, newValue) => {
        this.setState({ selectedTab: newValue });
         initialTab=false;
    }

    render() {
        const { selectedTab } = this.state;
        const { tabs, item, inputTypes, handleChanges, selectedItem, retrievingItems, handleDeleteItem } = this.props;
        const { handleTab } = this;
        if (retrievingItems)  initialTab=true
        return (
            <Paper square>
                <AppBar position="static">
                    <Tabs value={ initialTab ? 0 : selectedTab} onChange={handleTab} textColor="inherit" TabIndicatorProps={{ style: { background: 'white' } }}>
                        {tabs.map((tab, index) => (
                            <Tab label={tab} key={index}/>
                        ))}
                    </Tabs>
                </AppBar>
                <div style={{ height: '67vh' }}>
                    {retrievingItems && <Loading size={75} />}
                    {!retrievingItems && tabs.map((tab, index) => (
                        <TabPanel selectedTab={initialTab ? 0 : selectedTab} index={index} key={index}>
                            <ConfigTab
                                layoutKeys={this.props.layoutKeys}
                                tab={tab}
                                fields={item && item[tab]}
                                inputTypes={inputTypes[tab]}
                                selectedItem={selectedItem}
                                handleChanges={handleChanges}
                                handleDeleteItem={handleDeleteItem}
                            />
                        </TabPanel>
                    ))}
                </div>
            </Paper>
        );
    }
}

export default ConfigTabs;
