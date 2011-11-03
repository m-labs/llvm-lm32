import os
import sys

import componentinfo

from util import *

###

class LLVMProjectInfo(object):
    @staticmethod
    def load_infos_from_path(llvmbuild_source_root):
        # FIXME: Implement a simple subpath file list cache, so we don't restat
        # directories we have already traversed.

        # First, discover all the LLVMBuild.txt files.
        #
        # FIXME: We would like to use followlinks=True here, but that isn't
        # compatible with Python 2.4. Instead, we will either have to special
        # case projects we would expect to possibly be linked to, or implement
        # our own walk that can follow links. For now, it doesn't matter since
        # we haven't picked up the LLVMBuild system in any other LLVM projects.
        for dirpath,dirnames,filenames in os.walk(llvmbuild_source_root):
            # If there is no LLVMBuild.txt file in a directory, we don't recurse
            # past it. This is a simple way to prune our search, although it
            # makes it easy for users to add LLVMBuild.txt files in places they
            # won't be seen.
            if 'LLVMBuild.txt' not in filenames:
                del dirnames[:]
                continue

            # Otherwise, load the LLVMBuild file in this directory.
            assert dirpath.startswith(llvmbuild_source_root)
            subpath = '/' + dirpath[len(llvmbuild_source_root)+1:]
            llvmbuild_path = os.path.join(dirpath, 'LLVMBuild.txt')
            for info in componentinfo.load_from_path(llvmbuild_path, subpath):
                yield info

    @staticmethod
    def load_from_path(source_root, llvmbuild_source_root):
        infos = list(
            LLVMProjectInfo.load_infos_from_path(llvmbuild_source_root))

        return LLVMProjectInfo(source_root, infos)

    def __init__(self, source_root, component_infos):
        # Store our simple ivars.
        self.source_root = source_root
        self.component_infos = component_infos

        # Create the component info map and validate that component names are
        # unique.
        self.component_info_map = {}
        for ci in component_infos:
            existing = self.component_info_map.get(ci.name)
            if existing is not None:
                # We found a duplicate component name, report it and error out.
                fatal("found duplicate component %r (at %r and %r)" % (
                        ci.name, ci.subpath, existing.subpath))
            self.component_info_map[ci.name] = ci

        # Disallow 'all' as a component name, which is a special case.
        if 'all' in self.component_info_map:
            fatal("project is not allowed to define 'all' component")

        # Add the root component.
        if '$ROOT' in self.component_info_map:
            fatal("project is not allowed to define $ROOT component")
        self.component_info_map['$ROOT'] = componentinfo.GroupComponentInfo(
            '/', '$ROOT', None)
        self.component_infos.append(self.component_info_map['$ROOT'])

        # Topologically order the component information according to their
        # component references.
        def visit_component_info(ci, current_stack, current_set):
            # Check for a cycles.
            if ci in current_set:
                # We found a cycle, report it and error out.
                cycle_description = ' -> '.join(
                    '%r (%s)' % (ci.name, relation)
                    for relation,ci in current_stack)
                fatal("found cycle to %r after following: %s -> %s" % (
                        ci.name, cycle_description, ci.name))

            # If we have already visited this item, we are done.
            if ci not in components_to_visit:
                return

            # Otherwise, mark the component info as visited and traverse.
            components_to_visit.remove(ci)

            # Validate the parent reference, which we treat specially.
            if ci.parent is not None:
                parent = self.component_info_map.get(ci.parent)
                if parent is None:
                    fatal("component %r has invalid reference %r (via %r)" % (
                            ci.name, ci.parent, 'parent'))
                ci.set_parent_instance(parent)

            for relation,referent_name in ci.get_component_references():
                # Validate that the reference is ok.
                referent = self.component_info_map.get(referent_name)
                if referent is None:
                    fatal("component %r has invalid reference %r (via %r)" % (
                            ci.name, referent_name, relation))

                # Visit the reference.
                current_stack.append((relation,ci))
                current_set.add(ci)
                visit_component_info(referent, current_stack, current_set)
                current_set.remove(ci)
                current_stack.pop()

            # Finally, add the component info to the ordered list.
            self.ordered_component_infos.append(ci)

        # FIXME: We aren't actually correctly checking for cycles along the
        # parent edges. Haven't decided how I want to handle this -- I thought
        # about only checking cycles by relation type. If we do that, it falls
        # out easily. If we don't, we should special case the check.

        self.ordered_component_infos = []
        components_to_visit = set(component_infos)
        while components_to_visit:
            visit_component_info(iter(components_to_visit).next(), [], set())

        # Canonicalize children lists.
        for c in self.ordered_component_infos:
            c.children.sort(key = lambda c: c.name)

    def print_tree(self):
        def visit(node, depth = 0):
            print '%s%-40s (%s)' % ('  '*depth, node.name, node.type_name)
            for c in node.children:
                visit(c, depth + 1)
        visit(self.component_info_map['$ROOT'])

    def write_components(self, output_path):
        # Organize all the components by the directory their LLVMBuild file
        # should go in.
        info_basedir = {}
        for ci in self.component_infos:
            # Ignore the $ROOT component.
            if ci.parent is None:
                continue

            info_basedir[ci.subpath] = info_basedir.get(ci.subpath, []) + [ci]

        # Generate the build files.
        for subpath, infos in info_basedir.items():
            # Order the components by name to have a canonical ordering.
            infos.sort(key = lambda ci: ci.name)

            # Format the components into llvmbuild fragments.
            fragments = filter(None, [ci.get_llvmbuild_fragment()
                                      for ci in infos])
            if not fragments:
                continue

            assert subpath.startswith('/')
            directory_path = os.path.join(output_path, subpath[1:])

            # Create the directory if it does not already exist.
            if not os.path.exists(directory_path):
                os.makedirs(directory_path)

            # Create the LLVMBuild file.
            file_path = os.path.join(directory_path, 'LLVMBuild.txt')
            f = open(file_path, "w")

            # Write the header.
            header_fmt = ';===- %s %s-*- Conf -*--===;'
            header_name = '.' + os.path.join(subpath, 'LLVMBuild.txt')
            header_pad = '-' * (80 - len(header_fmt % (header_name, '')))
            header_string = header_fmt % (header_name, header_pad)
            print >>f, """\
%s
;
;                     The LLVM Compiler Infrastructure
;
; This file is distributed under the University of Illinois Open Source
; License. See LICENSE.TXT for details.
;
;===------------------------------------------------------------------------===;
;
; This is an LLVMBuild description file for the components in this subdirectory.
;
; For more information on the LLVMBuild system, please see:
;
;   http://llvm.org/docs/LLVMBuild.html
;
;===------------------------------------------------------------------------===;
""" % header_string

            for i,fragment in enumerate(fragments):
                print >>f, '[component_%d]' % i
                f.write(fragment)
                print >>f
            f.close()

    def write_library_table(self, output_path):
        # Write out the mapping from component names to required libraries.
        #
        # We do this in topological order so that we know we can append the
        # dependencies for added library groups.
        entries = {}
        for c in self.ordered_component_infos:
            # Only Library and LibraryGroup components are in the table.
            if c.type_name not in ('Library', 'LibraryGroup'):
                continue

            # Compute the llvm-config "component name". For historical reasons,
            # this is lowercased based on the library name.
            llvmconfig_component_name = c.get_llvmconfig_component_name()
            
            # Get the library name, or None for LibraryGroups.
            if c.type_name == 'LibraryGroup':
                library_name = None
            else:
                library_name = c.get_library_name()

            # Get the component names of all the required libraries.
            required_llvmconfig_component_names = [
                self.component_info_map[dep].get_llvmconfig_component_name()
                for dep in c.required_libraries]

            # Insert the entries for library groups we should add to.
            for dep in c.add_to_library_groups:
                entries[dep][2].append(llvmconfig_component_name)

            # Add the entry.
            entries[c.name] = (llvmconfig_component_name, library_name,
                               required_llvmconfig_component_names)

        # Convert to a list of entries and sort by name.
        entries = entries.values()

        # Create an 'all' pseudo component. We keep the dependency list small by
        # only listing entries that have no other dependents.
        root_entries = set(e[0] for e in entries)
        for _,_,deps in entries:
            root_entries -= set(deps)
        entries.append(('all', None, root_entries))

        entries.sort()

        # Compute the maximum number of required libraries, plus one so there is
        # always a sentinel.
        max_required_libraries = max(len(deps)
                                     for _,_,deps in entries) + 1

        # Write out the library table.
        f = open(output_path, 'w')
        print >>f, """\
//===- llvm-build generated file --------------------------------*- C++ -*-===//
//
// Component Library Depenedency Table
//
// Automatically generated file, do not edit!
//
//===----------------------------------------------------------------------===//
"""
        print >>f, 'struct AvailableComponent {'
        print >>f, '  /// The name of the component.'
        print >>f, '  const char *Name;'
        print >>f, ''
        print >>f, '  /// The name of the library for this component (or NULL).'
        print >>f, '  const char *Library;'
        print >>f, ''
        print >>f, '\
  /// The list of libraries required when linking this component.'
        print >>f, '  const char *RequiredLibraries[%d];' % (
            max_required_libraries)
        print >>f, '} AvailableComponents[%d] = {' % len(entries)
        for name,library_name,required_names in entries:
            if library_name is None:
                library_name_as_cstr = '0'
            else:
                # If we had a project level component, we could derive the
                # library prefix.
                library_name_as_cstr = '"libLLVM%s.a"' % library_name
            print >>f, '  { "%s", %s, { %s } },' % (
                name, library_name_as_cstr,
                ', '.join('"%s"' % dep
                          for dep in required_names))
        print >>f, '};'
        f.close()

def main():
    from optparse import OptionParser, OptionGroup
    parser = OptionParser("usage: %prog [options]")
    parser.add_option("", "--source-root", dest="source_root", metavar="PATH",
                      help="Path to the LLVM source (inferred if not given)",
                      action="store", default=None)
    parser.add_option("", "--print-tree", dest="print_tree",
                      help="Print out the project component tree [%default]",
                      action="store_true", default=False)
    parser.add_option("", "--write-llvmbuild", dest="write_llvmbuild",
                      help="Write out the LLVMBuild.txt files to PATH",
                      action="store", default=None, metavar="PATH")
    parser.add_option("", "--write-library-table",
                      dest="write_library_table", metavar="PATH",
                      help="Write the C++ library dependency table to PATH",
                      action="store", default=None)
    parser.add_option("", "--llvmbuild-source-root",
                      dest="llvmbuild_source_root",
                      help=(
            "If given, an alternate path to search for LLVMBuild.txt files"),
                      action="store", default=None, metavar="PATH")
    (opts, args) = parser.parse_args()

    # Determine the LLVM source path, if not given.
    source_root = opts.source_root
    if source_root:
        if not os.path.exists(os.path.join(source_root, 'lib', 'VMCore',
                                           'Function.cpp')):
            parser.error('invalid LLVM source root: %r' % source_root)
    else:
        llvmbuild_path = os.path.dirname(__file__)
        llvm_build_path = os.path.dirname(llvmbuild_path)
        utils_path = os.path.dirname(llvm_build_path)
        source_root = os.path.dirname(utils_path)
        if not os.path.exists(os.path.join(source_root, 'lib', 'VMCore',
                                           'Function.cpp')):
            parser.error('unable to infer LLVM source root, please specify')

    # Construct the LLVM project information.
    llvmbuild_source_root = opts.llvmbuild_source_root or source_root
    project_info = LLVMProjectInfo.load_from_path(
        source_root, llvmbuild_source_root)

    # Print the component tree, if requested.
    if opts.print_tree:
        project_info.print_tree()

    # Write out the components, if requested. This is useful for auto-upgrading
    # the schema.
    if opts.write_llvmbuild:
        project_info.write_components(opts.write_llvmbuild)

    # Write out the required librariy, if requested.
    if opts.write_library_table:
        project_info.write_library_table(opts.write_library_table)

if __name__=='__main__':
    main()
