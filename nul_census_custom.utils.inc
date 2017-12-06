<?php

/**
 * Search Taxonomy by field on the taxonomy
 * @param  string $vocabMachineName  machine name of the vocabulary
 * @param  string $fieldName         name of field to search
 * @param  string $valueOfField      value to match on
 * @return object                    term object
 */
function nul_census_custom_searchTaxonomyByField($vocabMachineName, $fieldName, $valueOfField) {
  $term = NULL;
  $query = new EntityFieldQuery();
  $vocabulary = taxonomy_vocabulary_machine_name_load($vocabMachineName);
  $query->entityCondition('entity_type', 'taxonomy_term')
    ->propertyCondition('vid', $vocabulary->vid)
    ->fieldCondition($fieldName, 'value', $valueOfField);
  $terms = $query->execute();
  if (!empty($terms['taxonomy_term'])) {
    foreach ($terms['taxonomy_term'] as $key => $termDetails) {
      $term = taxonomy_term_load($termDetails->tid);
    }
  }
  return $term;
}

/**
 * Check permissions of user to edit a tab or census based on status
 * @param  string $status status of tab
 * @param  string $contentType census or tab
 * @return boolean        can they edit or not
 */
function nul_census_custom_check_status_permisions($status, $contentType) {
  global $user;
  // IF user is an administrator they can edit whatever always
  if (in_array('administrator', $user->roles)) {
    $canEdit = TRUE;
  }
  // If user is not admin check permissions
  if (!in_array('administrator', $user->roles)) {
    if ($contentType == 'tab') {
      switch ($status) {
        case 'Incomplete':
          $canEdit = TRUE;
          break;

        case 'Complete':
          if (user_access('edit census tabs with status submitted reviewed complete or review complete')) {
            $canEdit = TRUE;
          }
          else {
            $canEdit = FALSE;
          }
          break;

        case 'Resubmit':
          if (user_access('edit census tabs with status resubmit')) {
            $canEdit = TRUE;
          }
          else {
            $canEdit = FALSE;
          }
          break;

        case 'Reviewed':
          if (user_access('edit census tabs with status submitted reviewed complete or review complete')) {
            $canEdit = TRUE;
          }
          else {
            $canEdit = FALSE;
          }
          break;

        case 'Reviewed Complete':
          if (user_access('edit census tabs with status submitted reviewed complete or review complete')) {
            $canEdit = TRUE;
          }
          else {
            $canEdit = FALSE;
          }
          break;

        case 'Submitted':
          if (user_access('edit census tabs with status submitted reviewed complete or review complete')) {
            $canEdit = TRUE;
          }
          else {
            $canEdit = FALSE;
          }
          break;

        default:
          $canEdit = FALSE;

          break;
      }
    }
    if ($contentType == 'census') {
      switch ($status) {
        case 'Incomplete':
          $canEdit = TRUE;
          break;

        case 'Complete':
          if (user_access('edit census with status complete')) {
            $canEdit = TRUE;
          }
          else {
            $canEdit = FALSE;
          }
          break;

        case 'Reviewed Complete':
          $canEdit = FALSE;
          break;

        case 'Submitted':
          if (user_access('edit census with status submitted')) {
            $canEdit = TRUE;
          }
          else {
            $canEdit = FALSE;
          }
          break;

        default:
          $canEdit = FALSE;

          break;
      }
    }
  }
  return $canEdit;
}

/**
 * fields required to be filled to move from incomplete to complete
 * @return array field machine names
 */
function nul_census_custom_fake_required_fields() {
  $fields = array(
    'field_advocacy_description' => 'civic_engagement',
    // machine names
  );
  return $fields;
}

/**
 * Checks related tabs statuses
 * @param int $nid parent census node id
 * @return array  array with one array of complete nodes and one array of machine names of not complete tab types
 */
function nul_census_custom_check_tab_statuses($nid, $exceptThisType) {
  $tabsReport = array(
    'complete' => array(),
    'notComplete' => array(),
  );
  // TODO check the status of the census before continuing to make sure we need to do all this
  $parentCensus = node_load($nid);
  // load census tab vocabulary
  $censusTabsVocabulary = taxonomy_vocabulary_machine_name_load('census_tabs');
  $censusTabs = taxonomy_get_tree($censusTabsVocabulary->vid);
  // foreach thru each term in census tab vocab
  foreach ($censusTabs as $key => $object) {
    if (!empty($object->tid)) {
      // load term
      $term = taxonomy_term_load($object->tid);
      // if there is a machine name content type for that term and it is not
      if (!empty($term->field_content_type_machine_name[LANGUAGE_NONE][0]['value'])) {
        if ($term->field_content_type_machine_name[LANGUAGE_NONE][0]['value'] !== $exceptThisType) {
          $completeTerm = taxonomy_get_term_by_name('Complete', 'census_tab_status');
          // search for node of that type AND parent census with NID passed to the function
          $query = new EntityFieldQuery();
          $query->entityCondition('entity_type', 'node')
            ->entityCondition('bundle', $term->field_content_type_machine_name[LANGUAGE_NONE][0]['value'])
            ->fieldCondition('field_parent_census', 'target_id', $nid)
            ->fieldCondition('field_tab_status', 'tid', key($completeTerm));
          $results = $query->execute();
          // If not found push machine name of not complete node
          if (empty($results['node'])) {
            $tabsReport['notComplete'][] = $term->field_content_type_machine_name[LANGUAGE_NONE][0]['value'];
          }
          // If found push data for complete tab to array
          if (!empty($results['node'])) {
            $tabsReport['complete'][key($results['node'])] = $results['node'][key($results['node'])];
          }
        }
      }
    }
  }
  return $tabsReport;
}

/**
 * update statuses of tabs
 * @param  array $tabsCompleted   array of tabs where key is the nid
 * @param  string $statusToChangeTo name of status to change to
 */
function nul_census_custom_update_related_tabs_status($tabsCompleted, $statusToChangeTo) {
  $submittedTabTerm = key(taxonomy_get_term_by_name($statusToChangeTo, 'census_tab_status'));
  foreach ($tabsCompleted as $nid => $object) {
    $tab = node_load($nid);
    $tab->field_tab_status[LANGUAGE_NONE][0]['tid'] = $submittedTabTerm;
    $tab = node_save($tab);
  }
}