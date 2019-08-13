# Faser Manager

The Faser manager is a Go web app that uses the Buffalo framework.

## Database Setup

### Configuration

Faser Manager is configured to use sqlite.

### Database creation

Releases should distribute a DB file.
In development, ask Buffalo to create the database:

	$ buffalo pop create -a

## Starting the Application

	$ make run

The app will run on [http://127.0.0.1:8888](http://127.0.0.1:8888).
