package require -exact vtkrendering 4.5

# We need Tk to implement the interactive features.
package require Tk

# Load the interaction features.
foreach s {Interactor bindings-rw bindings-iw bindings setget} {
  source [file join [file dirname [info script]] "${s}.tcl"]
}

package provide vtkinteraction 4.5
