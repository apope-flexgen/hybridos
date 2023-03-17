/* eslint-disable no-trailing-spaces */
// TODO: Remove after dev

import React from 'react';
import { withStyles } from 'tss-react/mui';

import ConfigHeader from './ConfigHeader';
import ConfigList from './ConfigList';
import ConfigItem from './ConfigItem';
import ConfigTabs from './ConfigTabs';

import {
    getDataForURI,
    setOrPutDataForURI,
} from '../../../AppConfig';

import { STYLES_UICONFIG } from '../../styles';

import Loading from '../Loading';

/*
    List of types:
    - string
    - boolean
    - array
    - object: 
        - multi
        - select
    
*/

const CURRENT_VERSION = "1.0.2";
const CURRENT_PAGES_CONFIG = require('./Configs/config.json');

class UIConfig extends React.Component {
    constructor() {
        super();
        this.state = {
            layoutKeys: null,
            selectedPage: 'none',
            selectedItem: null,
            selectedIndex: null,
            items: null,
            originalItems: null,
            hasChanged: false,
            pages: null,
            isLoading: true,
            retrievingItems: false,
        };
    }

    componentDidMount() {
        getDataForURI('/dbi/ui_config/pages')
            .then((response) => response.json())
            .then((response) => {
                // Check if config exists, otherwise populate dbi
                if (response.body === null || response.body.data === undefined || response.body === {} || response.body.version === undefined || response.body.version !== CURRENT_VERSION) {
                    this.setPages();
                } else {
                    this.setState({
                        pages: response.body.data,
                        isLoading: false,
                    });
                }
            })
            .catch((error) => {
                console.log(`ERROR in UI_CONFIG/componentDidMount: ${error}`);
            });
    }

    // Populate dbi with current config
    setPages = () => {
        let currentConfig = JSON.stringify(CURRENT_PAGES_CONFIG);
        setOrPutDataForURI('/dbi/ui_config/pages', currentConfig, 'post')
            .then((response) => response.json())
            .then((response) => {
                this.setState({
                    pages: CURRENT_PAGES_CONFIG.data,
                    isLoading: false
                });
            })
    }

    // TODO: Will have issues of nothing is there in DBI (janky fix is populate with empty objects before calling)
    fetchPage = (selectedPage) => {
        this.setState({ retrievingItems: true, selectedPage });
        getDataForURI('/dbi/ui_config/show_documents')
            .then((response) => response.json())
            .then((response) => {
                if (response.body.includes(selectedPage)) {
                    // Fetch items if config exists
                    getDataForURI(`/dbi/ui_config/${selectedPage}`)
                        .then((response) => response.json())
                        .then((response) => {
                            if (selectedPage !== 'layout') {
                                getDataForURI('dbi/ui_config/layout')
                                    .then((response) => response.json())
                                    .then((response) => {
                                        let layoutKeys = Object.values(response.body.data).map(x => x.info.key);
                                        this.setState({layoutKeys});
                                    })
                                    .catch((error) => {
                                        throw new Error(`ERROR in UI_CONFIG/fetchPage: ${error}`);
                                    });
                            }
                            // Kind of messy, body must be in format { "data": [] }
                            let data = Object.values(response.body.data);
                            this.setState({
                                retrievingItems: false,
                                items: data,
                                originalItems: data,
                                selectedIndex: null,
                                selectedItem: null,
                                hasChanged: false,
                            })
                        })
                        .catch((error) => {
                            throw new Error(`ERROR in UI_CONFIG/fetchPage: ${error}`);
                        });
                } else {
                    // Set default items if config does not exist
                    this.setState({
                        retrievingItems: false,
                        items: [],
                        originalItems: [],
                        hasChanged: false,
                    });
                }
            })
            .catch((error) => {
                throw new Error(`ERROR in UI_CONFIG/fetchPage/showdocuments: ${error}`);
            });
    }

    handlePageSelect = (event) => {
        if (event.target.value !== 'none') {
            this.fetchPage(event.target.value)
        } else {
            this.setState({
                selectedPage: 'none',
                selectedItem: null,
                selectedIndex: null,
                items: null,
                hasChanged: false,
                retrievingItems: false,
            });
        }
    }

    handleItemSelect = (item, index) => {
        if (this.state.selectedIndex === index) {
            this.setState({
                selectedItem: null,
                selectedIndex: null,
            });
        } else {
            this.setState({
                selectedItem: item,
                selectedIndex: index,
            });
        }
    }

    handleAddItem = (units, type) => {
        let itemsCopy = [...this.state.items];
        let newItem;
        if (type === 'duplicate') {
            newItem = JSON.parse(JSON.stringify(this.state.selectedItem));
        } else {
            newItem = {};
            for (const [keyA, valueA] of Object.entries(this.state.pages[this.state.selectedPage].inputTypes)) {
                let newObject;
                if (keyA === 'info' || keyA === 'alarms') {
                    newObject = {};
                    for (const [keyB, valueB] of Object.entries(valueA)) {
                        if (valueB === 'string') newObject[keyB] = keyB === 'name' ? 'New Item' : '';
                        if (valueB === 'boolean') newObject[keyB] = false;
                        if (valueB === 'array') newObject[keyB] = [];
                        if (valueB === 'count') newObject[keyB] = 0;
                        if (valueB === 'layoutKey') newObject[keyB] = '';
                        if (typeof valueB === 'object') {
                            if (valueB.type === 'multi') newObject[keyB] = [];
                            if (valueB.type === 'select') newObject[keyB] = null;
                        }
                    }
                } else newObject = [];

                newItem[keyA] = newObject;
            }
        }

        for (let i = 0; i < units; i++) itemsCopy.push(JSON.parse(JSON.stringify(newItem)));

        this.setState({
            items: itemsCopy,
            hasChanged: true,
        });
    }

    handleDeleteItem = () => {
        let itemsCopy = [...this.state.items];
        itemsCopy.splice(this.state.selectedIndex, 1);
        this.setState({
            selectedItem: null,
            selectedIndex: null,
            items: itemsCopy,
            hasChanged: true,
        })
    }

    // TODO: Will probably need to store original items (for reverting changes later)
    handleChanges = (tab, field, newValue, index) => {
        console.log('changed')
        let itemsCopy = [...this.state.items];
        if (index !== undefined) itemsCopy[this.state.selectedIndex][tab][index][field] = newValue;
        else if (field !== undefined) itemsCopy[this.state.selectedIndex][tab][field] = newValue;
        else itemsCopy[this.state.selectedIndex][tab] = newValue;
        this.setState({
            items: itemsCopy,
            hasChanged: true
        });
    }

    handleSave = (saving) => {
        if (saving) {
            let body = JSON.stringify({ data: this.state.items });
            setOrPutDataForURI(`/dbi/ui_config/${this.state.selectedPage}`, body, 'post')
                .then((response) => response.json())
                .then((response) => {
                    this.setState({
                        originalItems: this.state.items,
                        hasChanged: false,
                    })
                })
                .catch((error) => {
                    throw new Error(`ERROR in UI_CONFIG/handleSave: ${error}`);
                });
        } else {
            this.setState({
                selectedItem: null,
                selectedIndex: null,
                items: this.state.originalItems,
                hasChanged: false,
            })
        }
    }

    // TODO: Consider preventing switching page if changes have been made
    render() {
        const { handlePageSelect, handleAddItem, handleDeleteItem, handleItemSelect, handleChanges, handleSave } = this;
        const { selectedPage, selectedItem, selectedIndex, items, pages, hasChanged, isLoading, retrievingItems } = this.state;

        return (
            <React.Fragment>
                {isLoading && <Loading size={100} />}
                {!isLoading
                    && <React.Fragment>
                         <ConfigHeader
                            pages={pages}
                            selectedPage={selectedPage}
                            hasChanged={hasChanged}
                            handleSave={handleSave}
                            handlePageSelect={handlePageSelect}
                        /> 
                        <div className='Configuration-Components' style={{ display: 'flex', height: '72vh' }}>
                            <div className='Item-Selector' style={{ height: '100%', width: '500px', marginRight: '10px' }}>
                                <ConfigList
                                    items={items}
                                    nameOfItem={pages[selectedPage].nameOfItem}
                                    selectedIndex={selectedIndex}
                                    selectedItem={selectedItem}
                                    retrievingItems={retrievingItems}
                                    handleItemSelect={handleItemSelect}
                                />
                                <ConfigItem
                                    selectedPage={selectedPage}
                                    nameOfItem={pages[selectedPage].nameOfItem}
                                    selectedItem={selectedItem}
                                    retrievingItems={retrievingItems}
                                    handleAddItem={handleAddItem}
                                />
                            </div>
                            <div className='Item-Editor' style={{ width: '100%', minWidth: '60%', height: '100%' }}>
                                <ConfigTabs
                                    layoutKeys={this.state.layoutKeys}
                                    tabs={pages[selectedPage].tabs}
                                    item={typeof selectedIndex === 'number' ? items[selectedIndex] : null}
                                    inputTypes={pages[selectedPage].inputTypes}
                                    selectedItem={selectedItem}
                                    retrievingItems={retrievingItems}
                                    handleChanges={handleChanges}
                                    handleDeleteItem={handleDeleteItem}
                                />
                            </div>
                        </div>
                    </React.Fragment>
                }
            </React.Fragment>
        );
    }
}

export default withStyles(UIConfig, STYLES_UICONFIG);
