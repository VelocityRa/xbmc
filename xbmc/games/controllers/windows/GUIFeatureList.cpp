/*
 *      Copyright (C) 2014-2015 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "GUIFeatureList.h"
#include "GUIConfigurationWizard.h"
#include "GUIControllerDefines.h"
#include "GUIControllerWindow.h"
#include "games/controllers/guicontrols/GUIAnalogStickButton.h"
#include "games/controllers/guicontrols/GUIScalarFeatureButton.h"
#include "games/controllers/Controller.h"
#include "guilib/GUIButtonControl.h"
#include "guilib/GUIControlGroupList.h"
#include "guilib/GUIWindow.h"

using namespace GAME;

CGUIFeatureList::CGUIFeatureList(CGUIControllerWindow* window) :
  m_guiList(nullptr),
  m_guiButtonTemplate(nullptr),
  m_window(window)
{
  m_wizard = new CGUIConfigurationWizard;
}

CGUIFeatureList::~CGUIFeatureList(void)
{
  Deinitialize();
  delete m_wizard;
}

bool CGUIFeatureList::Initialize(void)
{
  m_guiList = dynamic_cast<CGUIControlGroupList*>(m_window->GetControl(CONTROL_FEATURE_LIST));
  m_guiButtonTemplate = dynamic_cast<CGUIButtonControl*>(m_window->GetControl(CONTROL_FEATURE_BUTTON_TEMPLATE));

  if (m_guiButtonTemplate)
    m_guiButtonTemplate->SetVisible(false);

  return m_guiList && m_guiButtonTemplate;
}

void CGUIFeatureList::Deinitialize(void)
{
  CleanupButtons();

  m_guiList = nullptr;
  m_guiButtonTemplate = nullptr;
}

void CGUIFeatureList::Load(const ControllerPtr& controller)
{
  if (m_controller && m_controller->ID() == controller->ID())
    return; // Already loaded

  CleanupButtons();

  m_controller = controller;

  const std::vector<CControllerFeature>& features = controller->Layout().Features();

  for (unsigned int buttonIndex = 0; buttonIndex < features.size(); buttonIndex++)
  {
    const CControllerFeature& feature = features[buttonIndex];

    CGUIButtonControl* pButton = nullptr;

    switch (feature.Type())
    {
      case JOYSTICK::FEATURE_TYPE::SCALAR:
      {
        pButton = new CGUIScalarFeatureButton(*m_guiButtonTemplate, m_wizard, feature, buttonIndex);
        break;
      }
      case JOYSTICK::FEATURE_TYPE::ANALOG_STICK:
      {
        pButton = new CGUIAnalogStickButton(*m_guiButtonTemplate, m_wizard, feature, buttonIndex);
        break;
      }
      default:
        break;
    }

    if (pButton)
      m_guiList->AddControl(pButton);

    // Just in case
    if (buttonIndex >= MAX_FEATURE_COUNT)
      break;
  }
}

void CGUIFeatureList::OnFocus(unsigned int index)
{
  IFeatureButton* button = GetButtonControl(index);
  if (button)
  {
    if (!m_wizard->IsPrompting(button))
      m_wizard->Abort();
  }
}

void CGUIFeatureList::OnSelect(unsigned int index)
{
  const unsigned int featureCount = m_controller->Layout().FeatureCount();

  std::vector<IFeatureButton*> buttons;

  for ( ; index < featureCount; index++)
  {
    IFeatureButton* control = GetButtonControl(index);
    if (!control)
      break;

    buttons.push_back(control);
  }

  m_wizard->Run(m_controller->ID(), buttons);
}

IFeatureButton* CGUIFeatureList::GetButtonControl(unsigned int featureIndex)
{
  CGUIControl* control = m_guiList->GetControl(CONTROL_FEATURE_BUTTONS_START + featureIndex);

  return dynamic_cast<CGUIFeatureButton*>(control);
}

void CGUIFeatureList::CleanupButtons(void)
{
  m_wizard->Abort(true);

  if (m_guiList)
    m_guiList->ClearAll();
}
