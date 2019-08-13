package actions

import (
	"faser_manager/models"
	"fmt"

	"github.com/gobuffalo/buffalo"
	"github.com/gobuffalo/pop"
)

// HomeHandler default implementation.
func HomeHandler(c buffalo.Context) error {
	// Get the DB connection from the context
	tx, ok := c.Value("tx").(*pop.Connection)
	if !ok {
		return fmt.Errorf("no transaction found")
	}

	pads := &models.Pads{}

	// Retrieve all Pads from the DB
	if err := tx.All(pads); err != nil {
		return err
	}

	c.Set("pads", pads)

	return c.Render(200, r.HTML("home.html"))
}
