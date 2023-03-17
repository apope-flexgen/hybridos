//Deprecated file

/* eslint-disable no-unused-expressions */
/* eslint-disable no-underscore-dangle */
import React from 'react';
import PropTypes from 'prop-types';
import { withStyles } from 'tss-react/mui';
import Grid from '@mui/material/Grid';
import SummaryCard from '../component/SummaryCard';
import { STYLES_FEATURES } from '../styles';
import { siteConfiguration } from '../../AppAuth';

const dashboardItems = [];

/**
 * Component that displays dashboard
 */
class Dashboard extends React.PureComponent {
    constructor(props) {
        super(props);
        this.state = {
            dashboardItems: [],
        };
        this._isMounted = false;
    }

    /**
     * Determine which dashboard items to display
     */
    componentDidMount() {
        this._isMounted = true;
        if (dashboardItems.length < 1) {
            if (siteConfiguration.includeSite) dashboardItems.push('site');
            if (siteConfiguration.includeFeatures) dashboardItems.push('features');
            if (siteConfiguration.includeStorage) dashboardItems.push('ess');
            if (siteConfiguration.includeGenerator) dashboardItems.push('generators');
            if (siteConfiguration.includeSolar) dashboardItems.push('solar');
            if (siteConfiguration.includeFeeders) dashboardItems.push('feeders');
            if (siteConfiguration.includeBMS) dashboardItems.push('bms');
            if (siteConfiguration.includePCS) dashboardItems.push('pcs');
            this._isMounted && this.setState({ dashboardItems });
        }
    }

    /**
     * Set isMounted to false
     */
    componentWillUnmount() {
        this._isMounted = false;
    }

    // eslint-disable-next-line class-methods-use-this
    render() {
        const { classes } = this.props;
        return (
            <>
                <Grid className={classes.root} container spacing={2}>
                    {dashboardItems.map((item, i) => <Grid key={i} item md={6}>
                        <SummaryCard type={item}/>
                    </Grid>)}
                </Grid>
            </>
        );
    }
}
Dashboard.propTypes = {
    classes: PropTypes.object,
};
export default withStyles(Dashboard, STYLES_FEATURES);
