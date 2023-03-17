/* eslint-disable react/prop-types */
import React, { Fragment } from 'react';
import { withStyles } from 'tss-react/mui';
import ComponentsPage from './ComponentsPage';
import { STYLES_FEATURES } from '../styles';
import DBIDownload from './DBIDownload';
import FIMSPage from './FIMSPage';

/**
 * Component for rendering inspector
 */
class InspectorPage extends React.PureComponent {
    constructor(props) {
        super(props);
        this.state = { ...this.state };
    }

    render() {
        return (
            <>
                <Fragment>
                    {this.props.type === 'dbiDownload' ? <DBIDownload /> : ''}
                    {this.props.type === 'components' ? <ComponentsPage asset_objects={{}} type='components' /> : ''}
                    {this.props.type === 'fims' ? <FIMSPage /> : ''}
                </Fragment>
            </>
        );
    }
}
export default withStyles(InspectorPage, STYLES_FEATURES);
