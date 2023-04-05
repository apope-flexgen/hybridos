import { Divider, Typography } from '@flexgen/storybook'
import { Box } from '@mui/material'
import Grid from '@mui/material/Grid'
import React, { ReactElement } from 'react'
import {
  AlertState,
  ConfigurableComponentFunction,
  ConfigurablePageStateStructure,
} from 'src/pages/ConfigurablePages/configurablePages.types'
import AlertContainer from './AlertContainer'
import { statusOuterBoxSx, statusPointsDisplayOuterBoxSx, statusPointsDisplaySubheaderBoxSx } from './assetsPage.styles'

// TODO: if we can find a way to calculate this automatically
// rather than setting a constant for everyone, that would be
// an upgrade. But this works for now
const DATAPOINTS_PER_COLUMN = 10
export interface SingleAssetProps {
  assetName: string
  statusChildren: ConfigurableComponentFunction[]
  assetState: ConfigurablePageStateStructure
  alertState: AlertState[string]
}

const Header = ({ headerText }: { headerText: string }) => (
  <Typography variant='headingL' color='primary' text={headerText} />
)

const StatusPointsDisplay = ({ statusComponents }: { statusComponents: ReactElement[] }) => {
  const numColumns = Math.ceil(statusComponents.length / DATAPOINTS_PER_COLUMN)
  return (
    <Box sx={statusPointsDisplayOuterBoxSx}>
      <Box sx={statusPointsDisplaySubheaderBoxSx}>
        <Typography variant='headingS' color='secondary' text='STATUS' />
        <Divider orientation='horizontal' variant='fullWidth' />
      </Box>
      <Grid container columns={numColumns} spacing={1}>
        {statusComponents}
      </Grid>
    </Box>
  )
}

const AssetStatus: React.FC<SingleAssetProps> = (props: SingleAssetProps): ReactElement => {
  const { assetName, statusChildren, assetState, alertState } = props

  const statusChildrenMapped = statusChildren.map((child) => (
    <Grid item xs={1}>
      {child(assetState)}
    </Grid>
  ))

  return (
    <>
      <Box
        sx={statusOuterBoxSx}
      >
        <Header headerText={assetName} />
        <AlertContainer alerts={alertState} />
        <StatusPointsDisplay statusComponents={statusChildrenMapped} />
      </Box>
    </>
  )
}

export default AssetStatus
