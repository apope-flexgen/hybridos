import LoginPowerLines from 'src/assets/LoginPowerLines.svg';
import styled from 'styled-components';

export interface LoginPageProps {
  loginComponent: JSX.Element;
}

const LoginPage = ({ loginComponent }: LoginPageProps): JSX.Element => {
  const LoginPageBackground = styled.div`
            background: linear-gradient(180deg, #404040 0%, #020202 100%);
            background-size: cover;
            min-height: 100%;
            min-width: 100%;
            position: absolute;
        `;
  const LoginPageGridLines = styled.div`
            background: url(${LoginPowerLines}) no-repeat center right;
            background-size: cover;
            min-height: 100%;
            min-width: 35%;
            right: 0;
            position: absolute;
        `;
  const LoginComponentContainer = styled.div`
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            overflow: auto;
        `;
  return (
    <LoginPageBackground>
      <LoginPageGridLines />
      <LoginComponentContainer>
        {loginComponent}
      </LoginComponentContainer>
    </LoginPageBackground>
  );
};

export default LoginPage;
