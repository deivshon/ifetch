<p><b>IMPORTANT: READ THE 'SAFETY INFORMATION' PARAGRAPH</b><p>
<hr>

<h2 align="center">ifetch</h2>

<p align="center"><img align="center" src="assets/images/default.png" width="450px" height="130px"></p>
<br>

ifetch is a simple tool for Linux systems to display network interface information. You can currently install it from source in a few easy steps, and soon using a number of package managers.

### <b>Installation and removal</b>

* Download the source files from the latest release and extract them from the archive (or directly clone the repository)
* <code>cd</code> into the ifetch directory
* Execute the command <code>make install</code> as root (using <code>sudo</code> or analogous tools) if you want to install ifetch
* Execute the command <code>make uninstall</code> as root if you want to uninstall ifetch and keep the configuration options in the <code>/etc/ifetch/</code> directory
* Execute the command <code>make purge</code> as root if you want to remove the configuration options in the <code>/etc/ifetch/</code> directory

### <b>Options</b>
Options are passed as arguments through the command line or extracted from a configuration file, necessarily named <code>ifetchrc</code>. Configuration files can be placed in <code>/etc/ifetch/</code> and <code>~/.config/ifetch/</code>, the latter takes precedence over the former.

While command line arguments have a standard <code>-{option} {value syntax}</code>, in configuration files the <code>{option}={value}</code> syntax is used.

If a configuration file is used **and** command line arguments are passed to ifetch, the command line arguments specified will take precedence over the corresponding ones in the configuration file. This means that a configuration file can be used and individual options can be specified through the command line to only change what is needed from the configuration file options.

Below is a cheatsheet displaying all currently available options, plus the data items and allowed colors table

| Option       | Allowed values              | Description                                                       |
|--------------|-----------------------------|-------------------------------------------------------------------|
| /            | Interface names             | Selects which interface to fetch data from                        |
| -if          | Interface names             | Selects which interface to fetch data from                        |
| -lo          | Logo file name<sup>1</sup>  | Selects the logo                                                  |
| -loc         | Color code                  | Selects the logo color                                            |
| -fc          | Color code                  | Selects the fields color                                          |
| -sc          | Color code                  | Selects the separator color                                       |
| -vc          | Color code                  | Selectes the values color                                         |
| -s           | String                      | Selects the separator                                             |
| -ns          | /                           | Selects a single space character as the separator                 |
| -mld         | Integer                     | Selects the minimum logo-field distance allowed                   |
| -mp          | Integer                     | Selects the minimum padding<sup>2</sup> allowed                   |
| -{...}l      | String                      | Selects the label for the specified data item                     |
| -{data item} | {show \| s \| hide \| h}    | Selects whether the specified data item will be showed or not     |

<sup>1</sup> Allowed logo file names are names of text files in either the <code>/etc/ifetch/logos/</code> or <code>~/.config/ifetch/logos/</code> directory. As with configuration files, the latter takes precedence over the former

<sup>2</sup> Padding here represents the distance between the logo and the last character of the data label (field)

| Data item | Description                                 |
|-----------|---------------------------------------------|
| if        | Interface name                              |
| mac       | MAC address of the selected interface       |
| ip4       | IPv4(s) assigned to the selected interface  |
| ip6       | IPv6(s) assigned to the selected interface  |
| rx        | Received bytes on the selected interface    |
| tx        | Transimtted bytes on the selected interface |

<img align="right" src="assets/images/arlecchino.png" width="450px" height="130px">

Options and data items can be mixed (except for the <code>-mld</code> and <code>-mp</code> options) to only have colors or separators on specific data rows. For instance, <code>-ifvc G</code> would make ifetch print the value (<code>vc</code>) of the <code>if</code> data item (interface name) in bold green. For more examples, take a look at <code>assets/configs/arlecchino_cfg</code>, the configuration file used to produce the output in the image above

| Color code | Color   |
|------------|---------|
| r          | Red     |
| g          | Green   |
| y          | Yellow  |
| b          | Blue    |
| m          | Magenta |
| c          | Cyan    |
| w          | White   |

Any of these color codes capitalized represents the bold version of the corresponding color

### <b>Safety information</b>
ifetch prints information relative to network interfaces, this includes the <b>IP addresses</b> they are assigned. These can be <b>private</b> or <b>public</b>, depending on your network setup.

<b>Make sure not to leak your public IP address(es)</b> if you are somehow sharing the output of ifetch (e.g. in a social media post, livestream, etc). If you don't know the difference between a public and private IP address, just hide the IP addresses data from the output using options in the configuration file or using the arguments passed through the command line.

I do not take any responsibility for any damage you might cause by leaking your public IP address(es) when sharing this software's output.
