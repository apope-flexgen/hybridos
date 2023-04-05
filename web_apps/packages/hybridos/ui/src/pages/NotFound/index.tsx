import { MuiButton, Typography } from '@flexgen/storybook';
import { useNavigate } from 'react-router-dom';
import PowerOff from 'src/assets/PowerOff.svg';
import { BACK_TO_DASHBOARD, DESCRIPTION_TEXT, PAGE_NOT_FOUND } from './helpers/constants';
import { ButtonSX, Container, Content } from './styles';

const NotFound = () => {
  const navigate = useNavigate();

  const handleClick = () => {
    navigate('/');
  };

  return (
    <Container>
      <img src={PowerOff} alt="Not found" />
      <Content>
        <Typography text={PAGE_NOT_FOUND} variant="bodyLBold" />
        <Typography text={DESCRIPTION_TEXT} variant="bodySLink" />
        <MuiButton
          variant="outlined"
          label={BACK_TO_DASHBOARD}
          onClick={handleClick}
          sx={ButtonSX}
        />
      </Content>
    </Container>
  );
};

export default NotFound;
