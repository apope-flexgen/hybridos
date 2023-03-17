import React from 'react';
import {
    Grid,
    Typography,
 } from '@mui/material';

import PropTypes from 'prop-types';
import { withStyles } from 'tss-react/mui';
import { STYLES_FLEET_MANAGER_DASHBOARD } from '../styles';
import ToggleButton from '@mui/material/ToggleButton';
import ToggleButtonGroup from '@mui/material/ToggleButtonGroup';

import { getDataForURI } from '../../AppConfig';

import FleetManagerSummaryCard from '../component/FleetManagerSummaryCard';
import TableView from '../component/DenseTableView/TableView';
import LoadingHOC from '../component/LoadingHOC';

class FleetManagerDashboard extends React.Component {
    constructor(props) {
        super(props);
        
        this.state = {
            cards: [],
            hasConfiguration: null,
            hasTemplateCards: false,
            toggleValue: 'table',
        }
        this.props.setLoading(true)
    }

    componentDidMount() {
        // Check if dashboard document exists
        getDataForURI('/dbi/ui_config/show_documents')
            .then((response) => response.json())
            .then((response) => {
                if (response.body.includes('dashboard')) {
                    // Get dashboard configuration
                    getDataForURI('/dbi/ui_config/dashboard')
                        .then((response) => response.json())
                        .then((response) => {
                            if (response.body.data && response.body.data.length !== 0) {
                                let hasTemplateCards = false;
                                for (const card of response.body.data) {
                                    if (card.info.isTemplate) {
                                        hasTemplateCards = true;
                                        break;
                                    }
                                }
                                this.props.setLoading(false)
                                this.setState({
                                    cards: response.body.data,
                                    hasConfiguration: true,
                                    hasTemplateCards
                                });
                            } else this.props.setLoading(false);
                        })
                } else {
                    this.props.setLoading(false);
                }
            })
            .catch((error) => {
                throw new Error(`ERROR in FleetmanagerDashboard/componentDidMount: ${error}`);
            })
    }

    //supports both template and non-template
    renderFMTable(card, index) {
        return (
            <React.Fragment>
                <TableView
                    info={card}
                />
            </React.Fragment>
        );
        
    }

    renderFMCard(card, index) {
        if (card.info.isTemplate) {
            return (
                <React.Fragment>
                    {card.info.items.map((item, index) => (
                        <FleetManagerSummaryCard
                            baseURI={item.uri}
                            name={item.name}
                            statuses={card.status}
                        />
                    ))}
                </React.Fragment>
            );
        } else {
            return (
                <FleetManagerSummaryCard
                    baseURI={card.info.baseURI}
                    name={card.info.name}
                    statuses={card.status}
                />
            );
        }
    }

    handleToggleChange = (event, newAlignment) => {
        this.setState({
          toggleValue: newAlignment,
        });
    }

    // hasNonTemplateCards() {
    //     if (this.state.cards.length == 0) {
    //         return true;
    //     }
    //     for (let i = 0; i < this.state.cards.length; i++) {
    //         if (!this.state.cards[i].info.isTemplate) {
    //             if (this.state.toggleValue == 'table') {
    //                 this.setState({
    //                     toggleValue: 'grid'
    //                 })
    //             }
    //             return true;
    //         }
    //     }
    //     return false;
    // }

    render() {
        const { hasConfiguration, cards, hasTemplateCards } = this.state;
        let isToggledToGrid = this.state.toggleValue == 'grid';

        const { classes } = this.props;

        return (
            
            <React.Fragment>
                    {hasConfiguration
                        ?  <React.Fragment>
                            {hasTemplateCards
                                && <ToggleButtonGroup
                                        style={{paddingBottom: '10px'}}
                                        color="primary"
                                        value={this.state.toggleValue}
                                        exclusive
                                        onChange={this.handleToggleChange}
                                    >
                                    <ToggleButton value="table">Table</ToggleButton>
                                    <ToggleButton value="grid">Grid</ToggleButton>
                                </ToggleButtonGroup>
                            }
                            {isToggledToGrid
                                ? <Grid container spacing={2}>
                                    {cards.map((card, index) => this.renderFMCard(card, index))}
                                </Grid>
                                : <React.Fragment>
                                    {cards.map((card, index) => this.renderFMTable(card, index))}
                                </React.Fragment>
                            }
                        </React.Fragment>
                        : <> {hasConfiguration==null && <Typography>Dashboard is not configured, please configure or contact an admin to configure the dashboard</Typography>} </>
                    }
            </React.Fragment>
        );
    }
}

FleetManagerDashboard.propTypes = {
    classes: PropTypes.object,
};
export default withStyles(LoadingHOC(FleetManagerDashboard), STYLES_FLEET_MANAGER_DASHBOARD);
