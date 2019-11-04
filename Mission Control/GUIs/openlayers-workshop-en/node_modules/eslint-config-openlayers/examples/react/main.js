import React, {Component} from 'react';
import {string} from 'prop-types';

class Widget extends Component {
  render() {
    return <div>Hello {this.props.name}!</div>;
  }
}

Widget.propTypes = {
  name: string.isRequired
};

export default Widget;
