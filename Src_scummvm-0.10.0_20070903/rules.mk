# Common build rules, used by the sub modules and their module.mk files

# Copy the list of objects to a new variable. The name of the new variable
# contains the module name, a trick we use so we can keep multiple different
# module object lists, one for each module.
MODULE_OBJS-$(MODULE) := $(addprefix $(MODULE)/, $(MODULE_OBJS))

# Add all involved directories to the MODULE_DIRS list
MODULE_DIRS += $(sort $(dir $(MODULE_OBJS-$(MODULE))))


ifdef PLUGIN
# Plugin build rule
# TODO: Right now, for Mac OS X only. We either will have to generate this
# via the configure script, or put in some 'if' statements to choose from
# one of several build rules
PLUGIN-$(MODULE) := plugins/$(PLUGIN_PREFIX)$(notdir $(MODULE))$(PLUGIN_SUFFIX)
$(PLUGIN-$(MODULE)): $(MODULE_OBJS-$(MODULE)) $(PLUGIN_EXTRA_DEPS)
	$(MKDIR) plugins
	$(CXX) $(filter-out $(PLUGIN_EXTRA_DEPS),$+) $(PLUGIN_LDFLAGS) -o $@
PLUGIN:=
plugins: $(PLUGIN-$(MODULE))

# Pseudo target for comfort, allows for "make common", "make gui" etc.
$(MODULE): $(PLUGIN-$(MODULE))

else

MODULE_LIB-$(MODULE) := $(MODULE)/lib$(notdir $(MODULE)).a

# If not building as a plugin, add the object files to the main OBJS list
OBJS += $(MODULE_LIB-$(MODULE))

# Convenience library target
$(MODULE_LIB-$(MODULE)): $(MODULE_OBJS-$(MODULE))
	-$(RM) $@
	$(AR) $@ $+
	$(RANLIB) $@ 

# Pseudo target for comfort, allows for "make common", "make gui" etc.
$(MODULE): $(MODULE_LIB-$(MODULE))

endif


# Clean target, removes all object files. This looks a bit hackish, as we have to
# copy the content of MODULE_OBJS to another unique variable (the next module.mk
# will overwrite it after all). The same for the libMODULE.a library file.
clean: clean-$(MODULE)
clean-$(MODULE): clean-% :
	-$(RM) $(MODULE_OBJS-$*) $(MODULE_LIB-$*) $(PLUGIN-$*)

.PHONY: clean-$(MODULE) $(MODULE)
