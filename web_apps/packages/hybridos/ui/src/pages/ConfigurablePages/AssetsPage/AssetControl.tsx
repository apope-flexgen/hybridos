import { ThemeType, Typography } from '@flexgen/storybook'
import { Box } from '@mui/material'
import {
  ConfigurablePageStateStructure,
  DisplayGroupFunctions,
} from '../configurablePages.types'
import { useTheme } from 'styled-components'
import { getControlOuterBoxSx, getControlInnerBoxSx } from './assetsPage.styles'

type AssetControlProps = {
  componentFunctions?: DisplayGroupFunctions
  assetState: ConfigurablePageStateStructure
}

const AssetControl = ({ componentFunctions, assetState }: AssetControlProps) => {
  const theme = useTheme() as ThemeType

  const controlChildrenMapped =
    componentFunctions !== undefined ? (
      componentFunctions.controlFunctions.map((child) => <>{child(assetState)}</>)
    ) : (
      <></>
    )

  return (
    <Box
      sx={getControlOuterBoxSx(theme)}
    >
      <Box sx={{ margin: '5px' }}>
        <Typography
          variant='headingS'
          text={
            componentFunctions?.displayName
              ? `${componentFunctions.displayName.toUpperCase()} CONTROLS`
              : 'Asset Controls'
          }
          color='secondary'
        />
      </Box>
      <Box
        sx={getControlInnerBoxSx(theme)}
      >
        {controlChildrenMapped}
      </Box>
    </Box>
  )
}

export default AssetControl
