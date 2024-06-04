# Start with the official Golang image
FROM golang:1.20 as builder

# Set the current working dir in container
WORKDIR /app

# Initialize pluto as its own go module
RUN go mod init github.com/flexgen-power/hybridos/pluto

# Copy source from current dir -> working dir
COPY . .

# Fetch dependencies
RUN go mod tidy

# Build the Go app
RUN CGO_ENABLED=0 GOOS=linux GOARCH=amd64 go build -o cmd/main .

# Run tests
RUN go test ./...

# Start a new stage from scratch
FROM flexgen/rocky9:latest


# Set the Current Working Directory inside the container
WORKDIR /home

# Copy the Pre-built binary file from the previous stage
COPY --from=builder /app/cmd/main .

# Command to run the executable
CMD ["./main -sayhello"]

# Continue commands here to run integration tests...
