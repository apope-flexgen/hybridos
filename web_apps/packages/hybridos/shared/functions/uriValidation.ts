export const validateURI = (uri: string) => {
  const regex = /^(?!\/\/)\/(?!.*\s)(?!.*\/\/).*$/
  return regex.test(uri)
}
